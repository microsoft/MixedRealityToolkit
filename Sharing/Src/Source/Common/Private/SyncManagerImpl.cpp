//////////////////////////////////////////////////////////////////////////
// SyncManagerImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncManagerImpl.h"
#include "CreateOperation.h"
#include "DeleteOperation.h"
#include "AckOperation.h"
#include <random>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

XTOOLS_REFLECTION_DEFINE(SyncManager)
.BaseClass<Reflection::XTObject>()
.BaseClass<IUpdateable>();

XTOOLS_REFLECTION_DEFINE(SyncManagerImpl)
.BaseClass<SyncManager>();


SyncManagerPtr SyncManager::Create(MessageID messageID, AuthorityLevel authorityLevel, const UserPtr& localUser)
{
	return new SyncManagerImpl(messageID, authorityLevel, localUser);
}


//////////////////////////////////////////////////////////////////////////
// Used to determine which elements to keep and which to delete when a remote
// client disconnects
class SyncManagerImpl::UserDiscriminator
{ 
public: 
	virtual bool operator()(UserID id) const = 0; 
};

// Returns true if the ID is a certain user
class SyncManagerImpl::OnlyUser : public UserDiscriminator
{ 
public:
	UserID m_id;
	OnlyUser(UserID id) : m_id(id) {}
	virtual bool operator()(UserID id) const
	{
		return (id == m_id);
	}
};

// Returns true for all but one user and the invalid user ID
// (The Invalid id is used to identify elements with no owner that should be shared)
class SyncManagerImpl::AllButUser : public UserDiscriminator
{
public:
	UserID m_id;
	AllButUser(UserID id) : m_id(id) {}
	virtual bool operator()(UserID id) const
	{
		return (id != m_id && id != User::kInvalidUserID);
	}
};


enum SyncMessages : byte
{
	IdentityInfo = 0,	// Contains the systemID and auth level for the remote machine
	SyncChanges			// Contains a list of changes to the shared state.
};


void SyncManagerImpl::RemoteSyncPeer::SendOp(const OperationConstPtr& op)
{
	VersionedOp opVersion(op, m_localState);

	m_sendList.push_back(opVersion);
	m_outgoing.push_back(opVersion);

	++m_localState.m_opsSent;
}


SyncManagerImpl::SyncManagerImpl(MessageID messageID, AuthorityLevel authorityLevel, const UserPtr& localUser)
	: m_listener(NULL)
	, m_messageID(messageID)
{
	XTASSERT(localUser);

	// Generate a random ID for this system on startup.  Ensure that it is not zero
	SystemID localSystemID = 0;
	do 
	{
		std::random_device randomGenerator;
		localSystemID = randomGenerator();
	} while (localSystemID == 0);
	
	m_syncContext = new SyncContext(authorityLevel, localSystemID, localUser);
}


bool SyncManagerImpl::RegisterListener(SyncListener* listener)
{
	if (listener)
	{
		// If we do not already have a listener...
		if (!m_listener)
		{
			// If the listener is not already registered...
			if (!listener->IsRegistered())
			{
				m_listener = listener;
				RegistrationReceiptPtr receipt = CreateRegistrationReceipt(SyncManagerImplPtr(this), &SyncManagerImpl::UnregisterListener, listener);
				m_listener->AddRegistration(receipt, receipt->GetKey());
				return true;
			}
			else
			{
				LogError("Trying to register a listener that has already been registered");
				return false;
			}
		}
		else
		{
			if (m_listener == listener)
			{
				LogWarning("SyncListener registration failed because the given listener is already registered");
			}
			else
			{
				LogWarning("SyncListener registration failed because another listener is already registered");
			}

			return false;
		}
	}
	else
	{
		LogError("Trying to register a NULL pointer");
		return false;
	}
}


void SyncManagerImpl::AddConnection(const NetworkConnectionPtr& newConnection)
{
#if defined(SYNC_DEBUG)
	LogInfo("%s Adding Connection", m_syncContext->GetLocalUser()->GetName().GetString().c_str());
#endif

	newConnection->AddListener(m_messageID, this);

	m_remoteHosts.resize(m_remoteHosts.size() + 1, RemoteSyncPeer());

	RemoteSyncPeer& newPeer = m_remoteHosts.back();
	newPeer.m_connection = newConnection;

	// Automatically remove this object as a listener when newPeer is destroyed
	newPeer.m_listenerReceipt = CreateRegistrationReceipt(newConnection, &NetworkConnection::RemoveListener, m_messageID, this);

	if (newConnection->IsConnected())
	{
		SendHandshakeMessage(newPeer);
	}
}


void SyncManagerImpl::RemoveConnection(const NetworkConnectionPtr& connection)
{
#if defined(SYNC_DEBUG)
	LogInfo("%s Removing Connection", m_syncContext->GetLocalUser()->GetName().GetString().c_str());
#endif

	size_t connectionIndex;
	if (GetIndexOfConnection(connection, connectionIndex))
	{
		{
			RemoteSyncPeer& removedPeer = m_remoteHosts[connectionIndex];
			if (removedPeer.m_bHandshakeComplete)
			{
				DeleteDataFromUser(removedPeer.m_userID);
			}
		}

		m_remoteHosts.erase(m_remoteHosts.begin() + connectionIndex);
	}
}


ObjectElementPtr SyncManagerImpl::GetRootObject()
{
	return m_syncContext->GetRootObject();
}


void SyncManagerImpl::Update()
{
	PROFILE_SCOPE("SyncManager Update");

	const OperationList& appliedOpList = m_syncContext->GetAppliedOperations();

	// Add all the locally applied changes to the outgoing queue of the remote peers
	if (!appliedOpList.empty())
	{
		for (size_t i = 0; i < appliedOpList.size(); ++i)
		{
			for (size_t j = 0; j < m_remoteHosts.size(); ++j)
			{
				RemoteSyncPeer& currentHost = m_remoteHosts[j];

				if (currentHost.m_bHandshakeComplete && currentHost.m_bHasAddedStartingState)
				{
					currentHost.SendOp(appliedOpList[i]);
				}
			}
		}

		// Clear out the list of locally applied operations
		m_syncContext->ClearAppliedOperations();
	}


	// Check to see if any new remote peers have connected since the last sync, and if so
	// add create operation for each element in our list to their outgoing queue
	for (size_t hostIndex = 0; hostIndex < m_remoteHosts.size(); ++hostIndex)
	{
		RemoteSyncPeer& currentHost = m_remoteHosts[hostIndex];

		if (currentHost.m_bHandshakeComplete && !currentHost.m_bHasAddedStartingState)
		{
#if defined(SYNC_DEBUG)
			LogInfo("%s Sending Deltas", m_syncContext->GetLocalUser()->GetName().GetString().c_str());
#endif
			// Check that the deltas are the first ops we are sending to the remote client
			XTASSERT(currentHost.m_localState.m_opsSent == 0);
			XTASSERT(currentHost.m_outgoing.empty());
			XTASSERT(currentHost.m_sendList.empty());

			AddCreateOpForElementRecurs(currentHost, m_syncContext->GetRootObject());
			currentHost.m_bHasAddedStartingState = true;
		}
	}

	
	const TransformManager& transfromMgr = m_syncContext->GetTransformManager();

	// Loop through all the incoming remote changes
	for (size_t hostIndex = 0; hostIndex < m_remoteHosts.size(); ++hostIndex)
	{
		RemoteSyncPeer& currentHost = m_remoteHosts[hostIndex];

		if (currentHost.m_bHandshakeComplete)
		{
			VersionedOpList& incomingOpList = currentHost.m_incoming;
			VersionedOpList& outgoingOpList = currentHost.m_outgoing;

			// For each incoming operation
			for (VersionedOpList::iterator opIt = incomingOpList.begin(); opIt != incomingOpList.end(); ++opIt)
			{
				VersionedOp& remoteMsg = *opIt;

				// Debugging hack
				if (remoteMsg.m_state.m_opsSent == 0)
				{
					// The first op we get should be a catch-up data create of the root
					XTASSERT(remoteMsg.m_op->GetType() == Operation::Create && remoteMsg.m_op->GetTargetGuid() == m_syncContext->GetRootObject()->GetGUID());
				}

#if defined(SYNC_DEBUG)
				LogInfo("%s received op (%s) from %s, version %i %i", m_syncContext->GetLocalUser()->GetName().GetString().c_str(), remoteMsg.m_op->GetOpDescription().c_str(), currentHost.m_userName.c_str(), remoteMsg.m_state.m_opsSent, remoteMsg.m_state.m_opsReceived);	
#endif

				// Discard the acknowledged operations, we don't need to transform against them anymore
				while (!outgoingOpList.empty() && outgoingOpList.front().m_state.m_opsSent < remoteMsg.m_state.m_opsReceived)
				{
#if defined(SYNC_DEBUG)
					LogInfo("  Invalidating %s op, version %i %i", outgoingOpList.front().m_op->GetTypeName(), outgoingOpList.front().m_state.m_opsSent, outgoingOpList.front().m_state.m_opsReceived);
#endif
					outgoingOpList.erase(outgoingOpList.begin());
				}

				if (remoteMsg.m_op->GetType() != Operation::Ack)
				{
					// Ensure that our starting state is the same as the one the message was sent from
					XTASSERT(remoteMsg.m_state.m_opsSent == currentHost.m_localState.m_opsReceived);

					// Transform the incoming ops by the changes that have not yet been acknowledged by the remote host
					for (VersionedOpList::iterator outgoingOpIt = outgoingOpList.begin(); outgoingOpIt != outgoingOpList.end(); ++outgoingOpIt)
					{
						VersionedOp& localMsg = *outgoingOpIt;

						TransformedPair transformResult = transfromMgr.Transform(localMsg.m_op, remoteMsg.m_op);

						localMsg.m_op = transformResult.m_localOpTransformed;
						remoteMsg.m_op = transformResult.m_remoteOpTransformed;

						// Early-out of the loop if the op was rendered invalid by the transformation
						if (remoteMsg.m_op->GetType() == Operation::Noop)
						{
							break;
						}
					}

					if (remoteMsg.m_op->GetType() != Operation::Noop)
					{
						// Const-cast so that we can apply the operation and change the auth level.  
						// All the rest of the code uses const for safety, as the ops should not change except when applied
						OperationPtr clonedOp = const_cast<Operation*>(remoteMsg.m_op.get());

						// Apply the transformed ops to the local data set
						clonedOp->Apply(m_syncContext);

						// Now that the op has been applied locally, it is set to the local system's authority level before
						// being queued to be passed to the other systems
						clonedOp->SetAuthorityLevel(m_syncContext->GetAuthorityLevel());

						// Add the op to the list of changes applied this Sync so that we can notify the listeners when all changes have been applied
						m_pendingNotifications.push(clonedOp);

						// Add the transformed op to the outgoing list for the other hosts
						for (size_t otherHostIndex = 0; otherHostIndex < m_remoteHosts.size(); ++otherHostIndex)
						{
							if (otherHostIndex != hostIndex)
							{
								RemoteSyncPeer& otherHost = m_remoteHosts[otherHostIndex];
								if (otherHost.m_bHandshakeComplete && otherHost.m_bHasAddedStartingState)
								{
									otherHost.SendOp(clonedOp);
								}
							}
						}
					}

					++currentHost.m_localState.m_opsReceived;
				}

				// Record what we know of the state of the remote machine
				currentHost.m_remoteState = remoteMsg.m_state;
			}

			incomingOpList.clear();
		}
	}

	// Send the changes in each remotePeer's outgoing op list.  
	// Nothing will be sent if the last message was not acknowledged.  
	for (size_t hostIndex = 0; hostIndex < m_remoteHosts.size(); ++hostIndex)
	{
		SendSyncMessage(m_remoteHosts[hostIndex]);
	}

	// Notify listeners about the remote changes
	if (!m_pendingNotifications.empty())
	{
		if (m_listener)
		{
			try
			{
				m_listener->OnSyncChangesBegin();
			}
			catch (...)
			{
				LogError("Exception occurred during OnSyncChangesBegin callback");
			}
		}

		while (!m_pendingNotifications.empty())
		{
			OperationPtr currentOp = m_pendingNotifications.front();

			try
			{
				currentOp->Notify(m_syncContext);
			}
			catch (...)
			{
				LogError("Exception occurred during Sync notification callback");
			}

			m_pendingNotifications.pop();
		}

		if (m_listener)
		{
			try
			{
				m_listener->OnSyncChangesEnd();
			}
			catch (...)
			{
				LogError("Exception occurred during OnSyncChangesEnd callback");
			}
		}
	}
}


const SyncContextPtr& SyncManagerImpl::GetContext() const
{
	return m_syncContext;
}


// For debugging, means there are no pending local changes, and no outstanding messages we are waiting for 
// from our remote peers
bool SyncManagerImpl::IsFullySynced() const
{
	if (!m_syncContext->GetAppliedOperations().empty())
	{
		return false;
	}

	for (size_t i = 0; i < m_remoteHosts.size(); ++i)
	{
		if ((m_remoteHosts[i].m_localState.m_opsSent - m_remoteHosts[i].m_remoteState.m_opsReceived) > 0 ||
			!m_remoteHosts[i].m_incoming.empty() ||
			m_remoteHosts[i].m_outgoing.size() > 2 ||
			!m_remoteHosts[i].m_bHandshakeComplete ||
			!m_remoteHosts[i].m_bHasAddedStartingState)
		{
			return false;
		}
	}

	return true;
}


const UserPtr& SyncManagerImpl::GetLocalUser() const
{
	return m_syncContext->GetLocalUser();
}


void SyncManagerImpl::PrintSyncData() const
{
	LogInfo("Sync data for user %s:", m_syncContext->GetLocalUser()->GetName()->GetString().c_str());

	for (size_t i = 0; i < m_remoteHosts.size(); ++i)
	{
		LogInfo("Connected to %s, local state %i %i, remote state %i %i",
			m_remoteHosts[i].m_userName.c_str(),
			m_remoteHosts[i].m_localState.m_opsSent,
			m_remoteHosts[i].m_localState.m_opsReceived,
			m_remoteHosts[i].m_remoteState.m_opsSent,
			m_remoteHosts[i].m_remoteState.m_opsReceived
			);
	}

	m_syncContext->PrintSyncDataTree();
}


void SyncManagerImpl::OnConnected(const NetworkConnectionPtr& connection)
{
#if defined(SYNC_DEBUG)
	LogInfo("Sync: OnConnected");
#endif

	size_t connectionIndex;
	if (GetIndexOfConnection(connection, connectionIndex))
	{
		RemoteSyncPeer& remotePeer = m_remoteHosts[connectionIndex];

		SendHandshakeMessage(remotePeer);
	}
}


void SyncManagerImpl::OnDisconnected(const NetworkConnectionPtr& connection)
{
#if defined(SYNC_DEBUG)
	LogInfo("Sync: OnDisconnected");
#endif

	size_t connectionIndex;
	if (GetIndexOfConnection(connection, connectionIndex))
	{
		UserID removedUser = m_remoteHosts[connectionIndex].m_userID;

		// Reset all the info for the remote peer in preparation for a re-connection
		RemoteSyncPeer& remotePeer = m_remoteHosts[connectionIndex];
		remotePeer.m_incoming.clear();
		remotePeer.m_outgoing.clear();
		remotePeer.m_sendList.clear();
		remotePeer.m_localState = OperationalState();
		remotePeer.m_remoteState = OperationalState();
		remotePeer.m_lastAcknowledgedVersion = 0;
		remotePeer.m_systemID = kUnknownSystemID;
		remotePeer.m_authorityLevel = AuthorityLevel::Unknown;
		remotePeer.m_userName.clear();
		remotePeer.m_userID = User::kInvalidUserID;
		remotePeer.m_bHandshakeComplete = false;
		remotePeer.m_bHasAddedStartingState = false;

		DeleteDataFromUser(removedUser);
	}
	else
	{
		XTASSERT(false);
	}
}


void SyncManagerImpl::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& msg)
{
	SyncMessages messageType = (SyncMessages)msg.ReadByte();
	switch (messageType)
	{
	case IdentityInfo:
		OnIdentityInfoReceived(connection, msg);
		break;

	case SyncChanges:
		OnSyncChangeReceived(connection, msg);
		break;
	}
}


void SyncManagerImpl::UnregisterListener(SyncListener* listener)
{
	XTASSERT(listener == m_listener);
	m_listener = NULL;
}


void SyncManagerImpl::OnIdentityInfoReceived(const NetworkConnectionPtr& connection, NetworkInMessage& msg)
{
	size_t connectionIndex;
	if (GetIndexOfConnection(connection, connectionIndex))
	{
#if defined(SYNC_DEBUG)
		LogInfo("Sync: Received Handshake");
#endif

		RemoteSyncPeer& remotePeer = m_remoteHosts[connectionIndex];
		remotePeer.m_authorityLevel = static_cast<AuthorityLevel>(msg.ReadByte());
		remotePeer.m_systemID = msg.ReadInt32();
		remotePeer.m_userName = msg.ReadStdString();
		remotePeer.m_userID = msg.ReadInt32();


		// Verify that this systemID is not already in use by another machine.  If so, force a disconnection
		// rather than risk data corruption, because GUIDS are based on SystemIDs
		bool duplicateSystemIDFound = (m_syncContext->GetLocalSystemID() == remotePeer.m_systemID);
		if (!duplicateSystemIDFound)
		{
			for (size_t i = 0; i < m_remoteHosts.size(); ++i)
			{
				if (i == connectionIndex) continue;

				if (m_remoteHosts[i].m_systemID == remotePeer.m_systemID)
				{
					duplicateSystemIDFound = true;
					break;
				}
			}
		}
		
		// The sync system will break down if two machines of equal authority level connect.  
		// So verify that they are different levels, or else disconnect
		bool authLevelsMatch = (remotePeer.m_authorityLevel == m_syncContext->GetAuthorityLevel());

		if (duplicateSystemIDFound || authLevelsMatch)
		{	
			// We're registered to receive disconnect callbacks, so we know that SyncManagerImpl::OnDisconnected()
			// will be called as a result of calling this. 
			if (authLevelsMatch)
			{
				LogError("Two machines with equal authority levels have connected: breaking the connection to avoid sync data corruption");
			}
			else
			{
				LogError("Duplicate system ID detected: breaking the connection to avoid sync data corruption");
			}
			XTASSERT(!duplicateSystemIDFound);
			connection->Disconnect();
		}

		// We should not be receiving a handshake message more than once
		XTASSERT(remotePeer.m_bHandshakeComplete == false);

		remotePeer.m_bHandshakeComplete = true;
	}
}


void SyncManagerImpl::OnSyncChangeReceived(const NetworkConnectionPtr& connection, NetworkInMessage& msg)
{
#if defined(SYNC_DEBUG)
	LogInfo("%s ReceivedSyncPacket", m_syncContext->GetLocalUser()->GetName().GetString().c_str());
#endif

	size_t connectionIndex;
	if (GetIndexOfConnection(connection, connectionIndex))
	{
		RemoteSyncPeer& remotePeer = m_remoteHosts[connectionIndex];

		if (XTVERIFY(remotePeer.m_bHandshakeComplete))
		{
			XTASSERT(remotePeer.m_systemID != kUnknownSystemID);

			// Read out the number of ops sent in this message
			int32 numOps = msg.ReadInt32();

			// Read out each of the ops
			for (int32 i = 0; i < numOps; ++i)
			{
				VersionedOp incomingVersionedOp;

				// Read the state that the remote machine was in before the op was applied
				incomingVersionedOp.m_state.m_opsSent = msg.ReadInt32();
				incomingVersionedOp.m_state.m_opsReceived = msg.ReadInt32();

				// Read the type of the op
				Operation::Type opType = (Operation::Type)msg.ReadByte();

				// Create an op of the appropriate type
				OperationPtr incomingOp = m_syncContext->GetOpFactory().Make(opType, remotePeer.m_authorityLevel);

				// Read the op data
				incomingOp->Deserialize(msg);

				incomingVersionedOp.m_op = incomingOp;

				// Add it to the list of incoming ops
				remotePeer.m_incoming.push_back(incomingVersionedOp);
			}
		}
	}
}


bool SyncManagerImpl::GetIndexOfConnection(const NetworkConnectionPtr& connection, size_t& indexOut) const
{
	for (size_t i = 0; i < m_remoteHosts.size(); ++i)
	{
		if (m_remoteHosts[i].m_connection == connection)
		{
			indexOut = i;
			return true;
		}
	}

	return false;
}


void SyncManagerImpl::SendSyncMessage(RemoteSyncPeer& remotePeer)
{
	if (remotePeer.m_bHandshakeComplete)
	{
		// If our list of ops to send is empty, and we have more than 2 ops that we have not acknowledged,
		// send a noop so the remote machine can release its outgoing queue and save some memory
		if (remotePeer.m_sendList.empty() && 
			remotePeer.m_lastAcknowledgedVersion < remotePeer.m_localState.m_opsReceived)
		{
			VersionedOp op;
			op.m_op = new AckOperation(AuthorityLevel::Unknown);
			op.m_state = remotePeer.m_localState;
			remotePeer.m_sendList.push_back(op);
		}

		if (!remotePeer.m_sendList.empty()) // We have ops to send
		{
#if defined(SYNC_DEBUG)
			LogInfo("Sending messages to %s", remotePeer.m_userName.c_str());
#endif

			NetworkOutMessagePtr msg = remotePeer.m_connection->CreateMessage(m_messageID);

			// Add the sub-type of the message
			msg->Write((byte)SyncChanges);

			// Write out the number of ops sent in this message
			msg->Write((int32)remotePeer.m_sendList.size());

			// Write out each of the ops
			for (VersionedOpList::iterator it = remotePeer.m_sendList.begin(); it != remotePeer.m_sendList.end(); ++it)
			{
				VersionedOp& currentOp = *it;

#if defined(SYNC_DEBUG)
				LogInfo(" - %s, version %i %i", currentOp.m_op->GetOpDescription().c_str(), currentOp.m_state.m_opsSent, currentOp.m_state.m_opsReceived);
#endif

				// Add our view of the messages sent and received
				msg->Write(currentOp.m_state.m_opsSent);
				msg->Write(currentOp.m_state.m_opsReceived);

				// Write the type of the op
				msg->Write((byte)currentOp.m_op->GetType());

				// Write the op data
				currentOp.m_op->Serialize(msg);

				// Record that we have acknowledged all the messages received
				remotePeer.m_lastAcknowledgedVersion = currentOp.m_state.m_opsReceived;
			}

			// Send the message
			remotePeer.m_connection->Send(msg);

			remotePeer.m_sendList.clear();
		}
	}
}


void SyncManagerImpl::SendHandshakeMessage(RemoteSyncPeer& remotePeer)
{
#if defined(SYNC_DEBUG)
	LogInfo("%s Sending Handshake", m_syncContext->GetLocalUser()->GetName().GetString().c_str());
#endif

	// We should not be sending a handshake message if its already complete
	XTASSERT(remotePeer.m_bHandshakeComplete == false);

	// Ask the remote machine to send over its systemID and its sync auth level
	NetworkOutMessagePtr identityMsg = remotePeer.m_connection->CreateMessage(m_messageID);
	identityMsg->Write((byte)IdentityInfo);
	identityMsg->Write((byte)m_syncContext->GetAuthorityLevel());
	identityMsg->Write(m_syncContext->GetLocalSystemID());

	identityMsg->Write(m_syncContext->GetLocalUser()->GetName());
	identityMsg->Write(m_syncContext->GetLocalUser()->GetID());

	remotePeer.m_connection->Send(identityMsg);
}


void SyncManagerImpl::AddCreateOpForElementRecurs(RemoteSyncPeer& peer, const ElementPtr& element)
{
	// Construct a CreateOperation for this element
	XGuid parentGuid = kInvalidXGuid;
	ElementPtr parent = element->GetParent();
	if (parent)
	{
		parentGuid = parent->GetGUID();
	}

	ObjectElementPtr objElement = ObjectElement::Cast(element);

	{
		UserID ownerID = User::kInvalidUserID;
		if (objElement)
		{
			ownerID = objElement->GetOwnerID();
		}

		CreateOperationPtr createOp = new CreateOperation(
			element->GetElementType(),
			element->GetName(),
			element->GetGUID(),
			parentGuid,
			ownerID,
			element->GetXValue(),
			m_syncContext->GetAuthorityLevel(),
			m_syncContext);

		// Add to the list of outgoing ops
		peer.SendOp(createOp);
	}

	// If this is an object element...
	
	if (objElement)
	{
		// Recurs to the children of this element
		for (int32 i = 0; i < objElement->GetElementCount(); ++i)
		{
			AddCreateOpForElementRecurs(peer, objElement->GetElementAt(i));
		}
	}
	else
	{
		// If the element is an array, add Insert operations for all its entries
		ArrayElement* arrayElement = reflection_cast<ArrayElement>(element);
		if (arrayElement != nullptr)
		{
			int32 numEntries = arrayElement->GetCount();
			for (int32 i = 0; i < numEntries; ++i)
			{
				InsertOperationPtr insertOp = new InsertOperation(
					element->GetGUID(),
					i,
					arrayElement->GetXValue(i),
					m_syncContext->GetAuthorityLevel(),
					m_syncContext);

				// Add to the list of outgoing ops
				peer.SendOp(insertOp);
			}
		}
	}
}


void SyncManagerImpl::DeleteDataFromUser(UserID userID)
{
#if defined(SYNC_DEBUG)
	LogInfo("%s Deleting user data", m_syncContext->GetLocalUser()->GetName()->GetString().c_str());
#endif

	// Slight HACK: the server needs to clear out the data owned by the disconnecting user.
	// But the desktop and baraboo need to clear out data from _everyone_ except themselves.  
	// So we use the auth level to indicate which response the system should use.  
	if (m_syncContext->GetAuthorityLevel() == AuthorityLevel::High)
	{
		if (XTVERIFY(userID != User::kInvalidUserID))
		{
			DeleteDataFromUserRecurs(m_syncContext->GetRootObject(), OnlyUser(userID));
		}
	}
	else if (m_syncContext->GetAuthorityLevel() == AuthorityLevel::Medium &&	// MSLICE
		userID == m_syncContext->GetLocalUser()->GetID())						// The disconnected user is the same as your own userID
	{
		// In this case, the viewer (baraboo) has disconnected from the desktop app.  
		// We may still be in a session and do not want to delete any data, so do nothing
	}
	else if (XTVERIFY(m_syncContext->GetLocalUser()))
	{
		DeleteDataFromUserRecurs(m_syncContext->GetRootObject(), AllButUser(m_syncContext->GetLocalUser()->GetID()));
	}
}


void SyncManagerImpl::DeleteDataFromUserRecurs(const ElementPtr& element, const UserDiscriminator& discriminator)
{
	ObjectElementPtr objectElement = ObjectElement::Cast(element);
	if (objectElement)
	{
		if (discriminator(objectElement->GetOwnerID()))
		{
#if defined(SYNC_DEBUG)
			LogInfo("%s Deleting object %s with owner %i", m_syncContext->GetLocalUser()->GetName().GetString().c_str(), objectElement->GetName().GetString().c_str(), objectElement->GetOwnerID());
#endif
			// Create a delete operation to represent this change
			DeleteOperationPtr deleteOp = new DeleteOperation(element->GetGUID(), element->GetParent()->GetGUID(), m_syncContext);

			// Execute the operation
			deleteOp->Apply(m_syncContext);

			// Add the op to the list of ops applied sync the last sync, so that remote machines 
			// are sent the delete operation
			m_syncContext->AddAppliedOperation(deleteOp);

			// Add the op to the list of pending notifications, to notify the user on this machine that the element has been deleted
			m_pendingNotifications.push(deleteOp);
		}
		else
		{
			// Recurs to the children of this element
			for (int32 i = objectElement->GetElementCount() - 1; i >= 0; --i)
			{
				DeleteDataFromUserRecurs(objectElement->GetElementAt(i), discriminator);
			}
		}
	}
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END

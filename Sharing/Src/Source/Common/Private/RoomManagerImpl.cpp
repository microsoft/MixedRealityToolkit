//////////////////////////////////////////////////////////////////////////
// RoomManagerImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoomManagerImpl.h"
#include "RoomMessageID.h"


XTOOLS_NAMESPACE_BEGIN

RoomManagerImpl::RoomManagerImpl(const ClientContextConstPtr& context)
	: m_context(context)
	, m_listenerList(RoomListenerList::Create())
	, m_bUploadInProgress(false)
{
	// Create the sync element representing this object.  
	// NOTE: the room manager should be constructed before a connection to another device is made, so there should never 
	// be another object with the same name already in the sync system
	m_element = m_context->GetInternalSyncManager()->GetRootObject()->CreateObjectElement(new XString("RoomMgr"), new XString("RoomManager"));
	XTASSERT(m_element);

	m_element->AddListener(this);

	// Add a listener for room messages from the server
	m_context->GetSessionConnection()->AddListener(MessageID::RoomAnchor, this);
}


void RoomManagerImpl::AddListener(RoomManagerListener* newListener)
{
	m_listenerList->AddListener(newListener);
}

 
void RoomManagerImpl::RemoveListener(RoomManagerListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


int32 RoomManagerImpl::GetRoomCount() const
{
	return (int32)m_roomList.size();
}


RoomPtr RoomManagerImpl::GetRoom(int32 index)
{
	if (index >= 0 && index < GetRoomCount())
	{
		return m_roomList[index];
	}
	else
	{
		LogError("Tried to access room at invalid index %i", index);
		return nullptr;
	}
}


RoomPtr RoomManagerImpl::GetCurrentRoom()
{
	return m_currentRoom;
}


RoomPtr RoomManagerImpl::CreateRoom(const XStringPtr& roomName, RoomID roomID, bool keepOpenWhenEmpty)
{
	// If the user is currently in a room, leave it first
	LeaveRoom();

	// Check that a room with the given ID does not already exist
	for (size_t i = 0; i < m_roomList.size(); ++i)
	{
		if (m_roomList[i]->GetID() == roomID)
		{
			LogError("Failed to create room with ID %lli: a room with that ID already exists", roomID);
			return nullptr;
		}
	}

	// Create a Room object for this element
	RoomImplPtr newRoom = new RoomImpl(m_listenerList, roomName, roomID, keepOpenWhenEmpty);

	// Make the name of the object element for the room be "Room<RoomID> to keep it unique
	std::string roomElementName = "Room" + std::to_string(roomID);

	if (newRoom->BindLocal(m_element, roomElementName, nullptr))
	{
		// Add it to the list of rooms
		m_roomList.push_back(newRoom);

		// Notify listeners that the new room was added
		m_listenerList->NotifyListeners(&RoomManagerListener::OnRoomAdded, newRoom);

		// Set the newly created room as the current room
		m_currentRoom = newRoom;

		// Add the local user to the room
		UserID userID = m_context->GetLocalUser()->GetID();
		newRoom->GetUserArray().Insert(0, userID);

		// Notify listeners that the we've joined the room
		m_listenerList->NotifyListeners(&RoomManagerListener::OnUserJoinedRoom, newRoom, userID);

		return newRoom;
	}
	else
	{
		return nullptr;
	}
}


bool RoomManagerImpl::JoinRoom(const RoomPtr& room)
{
	// Check that we aren't already in the room
	if (room == m_currentRoom)
	{
		LogWarning("Trying to join a room that you are already in");
		return false;
	}

	// Leave the current room
	LeaveRoom();

	// Validate that the room passed in is in the list of rooms we know about
	XTASSERT(m_currentRoom == nullptr);
	for (size_t i = 0; i < m_roomList.size(); ++i)
	{
		if (m_roomList[i] == room)
		{
			// Set the new room as the current room
			m_currentRoom = m_roomList[i];
			break;
		}
	}

	if (m_currentRoom)
	{
		// Add the local user to the room
		UserID userID = m_context->GetLocalUser()->GetID();
		m_currentRoom->GetUserArray().Insert(m_currentRoom->GetUserCount(), userID);

		// Notify listeners that we've joined the room
		m_listenerList->NotifyListeners(&RoomManagerListener::OnUserJoinedRoom, m_currentRoom, userID);

		return true;
	}
	else
	{
		LogError("Attempting to join an invalid room");
		return false;
	}
}


bool RoomManagerImpl::LeaveRoom()
{
	// Always clear out any pending download request when this function is called
	m_currentDownloadRequest = nullptr;

	if (m_currentRoom != nullptr)
	{
		const UserID localUserID = m_context->GetLocalUser()->GetID();

		const int32 userCount = m_currentRoom->GetUserCount();
		for (int32 i = 0; i < userCount; ++i)
		{
			if (m_currentRoom->GetUserID(i) == localUserID)
			{
				m_currentRoom->GetUserArray().Remove(i);

				m_listenerList->NotifyListeners(&RoomManagerListener::OnUserLeftRoom, m_currentRoom, localUserID);

				break;
			}
		}

		RoomPtr leavingRoom = m_currentRoom;
		m_currentRoom = nullptr;

		// If we are working disconnected from a session and we left the room, and the room isn't marked for keeping, then delete it
		if (!m_context->GetSessionConnection()->IsConnected() &&
			!leavingRoom->GetKeepOpen()
			)
		{
			for (size_t i = 0; i < m_roomList.size(); ++i)
			{
				if (m_roomList[i]->GetID() == leavingRoom->GetID())
				{
					m_roomList.erase(m_roomList.begin() + i);

					m_listenerList->NotifyListeners(&RoomManagerListener::OnRoomClosed, leavingRoom);

					break;
				}
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}


bool RoomManagerImpl::DownloadAnchor(const RoomPtr& room, const XStringPtr& anchorName)
{
	if (m_currentDownloadRequest != nullptr)
	{
		LogError("Anchor download already in progress");
		return false;
	}

	// Create a new object to represent this request
	m_currentDownloadRequest = new AnchorDownloadRequestImpl(anchorName, room);

	// Build the request message and send it
	NetworkConnectionPtr serverConnection = m_context->GetSessionConnection();

	NetworkOutMessagePtr outMsg = serverConnection->CreateMessage(MessageID::RoomAnchor);

	outMsg->Write((byte)RoomMessageID::AnchorDownloadRequest);
	outMsg->Write(room->GetID());
	outMsg->Write(anchorName);
	serverConnection->Send(outMsg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::RoomAnchorChannel, true);

	return true;
}

 
bool RoomManagerImpl::UploadAnchor(const RoomPtr& room, const XStringPtr& anchorName, const byte* data, int32 dataSize)
{
	if (data == nullptr)
	{
		LogError("Anchor data passed to UploadAnchor is invalid");
		return false;
	}

	if (dataSize <= 0)
	{
		LogError("Anchor data saize passed to UploadAnchor is invalid");
		return false;
	}

	if (m_bUploadInProgress)
	{
		LogError("Anchor upload already in progress");
		return false;
	}

	NetworkConnectionPtr serverConnection = m_context->GetSessionConnection();

	if (!serverConnection->IsConnected())
	{
		LogError("Cannot upload anchors when not joined to a session");
		return false;
	}

	m_bUploadInProgress = true;

	// Build the request message and send it
	NetworkOutMessagePtr outMsg = serverConnection->CreateMessage(MessageID::RoomAnchor);
	outMsg->Write((byte)RoomMessageID::AnchorUploadRequest);
	outMsg->Write(room->GetID());
	outMsg->Write(anchorName);
	outMsg->Write(dataSize);
	outMsg->WriteArray(data, dataSize);
	serverConnection->Send(outMsg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::RoomAnchorChannel, true);

	return true;
}


void RoomManagerImpl::OnElementAdded(const ElementPtr& element)
{
	ObjectElementPtr objElement = ObjectElement::Cast(element);
	if (objElement && objElement->GetObjectType()->GetString() == "Room")
	{
		// Create a Room object for this element
		RoomImplPtr newRoom = new RoomImpl(m_listenerList);
		newRoom->BindRemote(element);

		// Add it to the list of rooms
		m_roomList.push_back(newRoom);

		// Notify the listeners about the new room
		m_listenerList->NotifyListeners(&RoomManagerListener::OnRoomAdded, newRoom);
	}
}


void RoomManagerImpl::OnElementDeleted(const ElementPtr& element)
{
	// Find the room that was deleted
	RoomImplPtr closedRoom = nullptr;

	for (size_t i = 0; i < m_roomList.size(); ++i)
	{
		if (m_roomList[i]->GetGUID() == element->GetGUID())
		{
			if (m_currentRoom != nullptr &&
				m_currentRoom->GetID() == m_roomList[i]->GetID())
			{
				m_currentRoom = nullptr;
			}

			closedRoom = m_roomList[i];
			m_roomList.erase(m_roomList.begin() + i);
			break;
		}
	}

	if (closedRoom != nullptr)
	{
		m_listenerList->NotifyListeners(&RoomManagerListener::OnRoomClosed, closedRoom);
	}
}


void RoomManagerImpl::OnDisconnected(const NetworkConnectionPtr& )
{
	m_bUploadInProgress = false;
	m_currentDownloadRequest = nullptr;

	// When we disconnect from the server, remove all open rooms except the one the user is in and any rooms marked as keepOpen

	// Iterate backwards because we will be removing items from the list as we iterate
	for (int32 i = (int32)m_roomList.size() - 1; i >= 0; --i)
	{
		// Clear out all anchors; only the server holds on to the anchor data
		m_roomList[i]->ClearAnchors();

		bool isCurrentRoom = 
			(m_currentRoom != nullptr &&
			m_currentRoom->GetID() == m_roomList[i]->GetID());

		if (!isCurrentRoom && !m_roomList[i]->GetKeepOpen())
		{
			RoomImplPtr closedRoom = m_roomList[i];
			m_roomList.erase(m_roomList.begin() + i);

			m_listenerList->NotifyListeners(&RoomManagerListener::OnRoomClosed, closedRoom);
		}
	}
}


void RoomManagerImpl::OnMessageReceived(const NetworkConnectionPtr& , NetworkInMessage& message)
{
	RoomMessageID msgType = (RoomMessageID)message.ReadByte();

	switch (msgType)
	{
	case XTools::AnchorUploadResponse:
		OnUploadResponse(message);
		break;

	case XTools::AnchorDownloadResponse:
		OnDownloadResponse(message);
		break;

	case XTools::AnchorsChangedNotification:
		OnAnchorsChanged(message);
		break;

	default:
		break;
	}
}


void RoomManagerImpl::OnUploadResponse(NetworkInMessage& message)
{
	bool bSuccess = (message.ReadByte() != 0);
	XStringPtr failureReason;

	if (!bSuccess)
	{
		failureReason = new XString(message.ReadStdString());
	}

	m_bUploadInProgress = false;

	m_listenerList->NotifyListeners(&RoomManagerListener::OnAnchorUploadComplete, bSuccess, failureReason);
}


void RoomManagerImpl::OnDownloadResponse(NetworkInMessage& message)
{
	PROFILE_FUNCTION();

	// If we are not currently waiting for a download then do not bother to process the message
	if (m_currentDownloadRequest == nullptr)
	{
		return;
	}

	bool bSuccess = (message.ReadByte() != 0);
	XStringPtr failureReason;

	if (bSuccess)
	{
		const int32 dataSize = message.ReadInt32();

		BufferPtr buffer(new Buffer(dataSize));
		buffer->Resize(dataSize);

		Profile::BeginRange("ReadArray");
		message.ReadArray(buffer->GetData(), buffer->GetSize());
		Profile::EndRange();

		m_currentDownloadRequest->SetData(buffer);
	}
	else
	{
		failureReason = new XString(message.ReadStdString());
	}

	// Clear the current request, so that the user can start a new request as part of the callback if they'd like
	AnchorDownloadRequestPtr request = m_currentDownloadRequest;
	m_currentDownloadRequest = nullptr;

	Profile::BeginRange("Notify Listeners");
	m_listenerList->NotifyListeners(&RoomManagerListener::OnAnchorsDownloaded, bSuccess, request, failureReason);
	Profile::EndRange();
}


void RoomManagerImpl::OnAnchorsChanged(NetworkInMessage& message)
{
	RoomID roomID = message.ReadInt64();

	RoomPtr room = nullptr;
	for (size_t i = 0; i < m_roomList.size(); ++i)
	{
		if (m_roomList[i]->GetID() == roomID)
		{
			room = m_roomList[i];
			break;
		}
	}

	if (room)
	{
		m_listenerList->NotifyListeners(&RoomManagerListener::OnAnchorsChanged, room);
	}
}

XTOOLS_NAMESPACE_END

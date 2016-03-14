//////////////////////////////////////////////////////////////////////////
// PairingManagerImpl.cpp
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PairingManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(PairingManager)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(PairingManagerImpl)
.BaseClass<PairingManager>()
.BaseClass<IUpdateable>();


PairingManagerImpl::PairingManagerImpl(const ClientContextConstPtr& context)
	: m_context(context)
	, m_bPairingInProgress(false)
{

}


bool PairingManagerImpl::HasPairingInfo() const
{
	// TODO: implement
	return false;
}


void PairingManagerImpl::ClearPairingInfo()
{
	// TODO: implement
}


bool PairingManagerImpl::BeginConnecting(PairingListener* )
{
	// TODO: implement
	return false;
}


void PairingManagerImpl::CancelConnecting()
{
	// TODO: implement
}


PairingResult PairingManagerImpl::BeginPairing(const PairMakerPtr& pairMaker, PairingListener* listener)
{
	if (m_bPairingInProgress)
	{
		return PairingResult::PairingAlreadyInProgress;
	}

	// First, clear out any existing pairing info
	ClearPairingInfo();

	m_pairMaker = pairMaker;
	m_listenerList = ListenerList::Create();
	m_listenerList->AddListener(listener);

	if (m_pairMaker->IsReadyToConnect())
	{
		PairingResult result = SetupConnection();
		if (result != PairingResult::Ok)
		{
			// Failed to setup the pairing attempt
			TeardownPairing(false, result);
			return result;
		}
	}

	return PairingResult::Ok;
}


void PairingManagerImpl::CancelPairing()
{
	if (m_bPairingInProgress)
	{
		// Make sure we're ready to make another pairing attempt before we notify the listener
		TeardownPairing(true, PairingResult::CanceledByUser);
	}
}


bool PairingManagerImpl::IsPairing() const
{
	return m_bPairingInProgress;
}


bool PairingManagerImpl::IsConnected() const
{
	return m_context->GetPairedConnection()->IsConnected();
}


void PairingManagerImpl::Update()
{
	// If the pair maker is ready to connect, but we haven't started pairing yet, then kick off the pairing process
	if (m_pairMaker && m_pairMaker->IsReadyToConnect() && !m_bPairingInProgress)
	{
		PairingResult result = SetupConnection();
		if (result != PairingResult::Ok)
		{
			TeardownPairing(true, result);
		}
	}
}


PairingResult PairingManagerImpl::SetupConnection()
{
	if (m_pairMaker->IsReceiver())
	{
		return SetupReceiverConnection();
	}
	else
	{
		return SetupConnectorConnection();
	}
}


PairingResult PairingManagerImpl::SetupReceiverConnection()
{
	// Start listening for incoming pairing connections.  When a new connection is received, OnNewConnection will be called
	m_pairedIncomingConnectionReceipt = m_context->GetXSocketManager()->AcceptConnections(m_pairMaker->GetPort(), 1, this);
	if (!m_pairedIncomingConnectionReceipt)
	{
		return PairingResult::FailedToOpenIncomingConnection;
	}

	m_bPairingInProgress = true;

	LogInfo("Listening for connections on port %i", m_pairMaker->GetPort());

	return PairingResult::Ok;
}


PairingResult PairingManagerImpl::SetupConnectorConnection()
{
	int32 addressCount = m_pairMaker->GetAddressCount();
	if (addressCount == 0)
	{
		return PairingResult::NoAddressToConnectTo;
	}

	for (int i = 0; i < m_pairMaker->GetAddressCount(); i++)
	{
		PairingResult attemptResult = BeginPairingAttempt(i);
		if (attemptResult != PairingResult::Ok)
		{
			return attemptResult;
}
	}

	return PairingResult::Ok;
}


PairingResult PairingManagerImpl::BeginPairingAttempt(int index)
{
	// Initiate a connection attempt to the remote client
	XSocketPtr viewerSocket = m_context->GetXSocketManager()->OpenConnection(m_pairMaker->GetAddress(index)->GetString(), m_pairMaker->GetPort());
	if (!viewerSocket)
	{
		return PairingResult::FailedToOpenOutgoingConnection;
	}

	LogInfo("Attempting to connect to remote client at %s:%i", m_pairMaker->GetAddress(index)->GetString().c_str(), m_pairMaker->GetPort());

	// Initiate the handshake
	HandshakeCallback callback = CreateCallback3(this, &PairingManagerImpl::OnHandshakeComplete);
	m_pairingHandshakes.insert(std::pair<SocketID, NetworkHandshakePtr>(viewerSocket->GetID(), new NetworkHandshake(viewerSocket, new PairingHandshakeLogic(m_context->GetLocalUser(), false), callback)));

	m_bPairingInProgress = true;

	return PairingResult::Ok;
}


void PairingManagerImpl::OnNewConnection(const XSocketPtr& newSocket)
{
	// Initiate the handshake
	HandshakeCallback callback = CreateCallback3(this, &PairingManagerImpl::OnHandshakeComplete);

	m_pairingHandshakes.insert(std::pair<SocketID, NetworkHandshakePtr>(newSocket->GetID(), new NetworkHandshake(newSocket, new PairingHandshakeLogic(m_context->GetLocalUser(), true), callback)));
}


void PairingManagerImpl::OnHandshakeComplete(const XSocketPtr& newSocket, SocketID socketId, HandshakeResult result)
{
	// Handshake complete.  If successful set the connection. 
	if (result == HandshakeResult::Success)
	{
		XTASSERT(newSocket != nullptr);

		m_context->GetPairedConnection()->SetSocket(newSocket);

		if (!m_context->IsPrimaryClient())
		{
			m_context->GetUserPresenceManager()->SetUser(m_context->GetLocalUser());
		}

		TeardownPairing(true, PairingResult::Ok);
	}
	else
	{
		// We didn't connect.  If this is the connector, try the next address
		m_pairingHandshakes.erase(socketId);
		if (m_pairingHandshakes.size() == 0)
		{
			// Failed connecting to all addresses.  Notify the user
			TeardownPairing(true, PairingResult::ConnectionFailed);
		}
	}
}


void PairingManagerImpl::TeardownPairing(bool bNotifyListener, PairingResult notifyResult)
{
	ListenerListPtr listeners = m_listenerList;

	m_pairMaker = nullptr;
	m_listenerList = nullptr;

	m_pairingHandshakes.clear();
	m_pairedIncomingConnectionReceipt = nullptr;

	m_bPairingInProgress = false;

	if (bNotifyListener && listeners)
	{
		if (notifyResult == PairingResult::Ok)
		{
			listeners->NotifyListeners(&PairingListener::PairingConnectionSucceeded);
		}
		else
		{
			listeners->NotifyListeners(&PairingListener::PairingConnectionFailed, notifyResult);
		}
	}
}

XTOOLS_NAMESPACE_END
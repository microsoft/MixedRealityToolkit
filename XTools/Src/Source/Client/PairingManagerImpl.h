//////////////////////////////////////////////////////////////////////////
// PairingManagerImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Implementation of the PairingManager interface, managing the creation
/// and storage of pairings to remote clients.  
class PairingManagerImpl : public PairingManager, public IUpdateable, public IncomingXSocketListener
{
	XTOOLS_REFLECTION_DECLARE(PairingManagerImpl)

public:
	explicit PairingManagerImpl(const ClientContextConstPtr& context);

	// PairingManager Functions:
	virtual bool			HasPairingInfo() const XTOVERRIDE;
	virtual void			ClearPairingInfo() XTOVERRIDE;

	virtual bool			BeginConnecting(PairingListener* listener) XTOVERRIDE;
	virtual void			CancelConnecting() XTOVERRIDE;

	virtual PairingResult	BeginPairing(const PairMakerPtr& pairMaker, PairingListener* listener) XTOVERRIDE;
	virtual void			CancelPairing() XTOVERRIDE;
	virtual bool			IsPairing() const XTOVERRIDE;

	virtual bool			IsConnected() const XTOVERRIDE;

	// IUpdateable Functions:
	virtual void			Update() XTOVERRIDE;

private:
	
	// IncomingXSocketListener Functions:
	virtual void			OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE;

	// Local Functions:
	PairingResult			SetupConnection();
	PairingResult			SetupReceiverConnection();
	PairingResult			SetupConnectorConnection();

	PairingResult			BeginPairingAttempt(int index);

	void					OnHandshakeComplete(const XSocketPtr& newSocket, SocketID, HandshakeResult result);
	void					TeardownPairing(bool bNotifyListener, PairingResult notifyResult);

	typedef ListenerList<PairingListener> ListenerList;
	DECLARE_PTR(ListenerList);

	ClientContextConstPtr	m_context;
	PairMakerPtr			m_pairMaker;
	ListenerListPtr			m_listenerList;
	std::map<SocketID, NetworkHandshakePtr> m_pairingHandshakes;
	ReceiptPtr				m_pairedIncomingConnectionReceipt;
	bool					m_bPairingInProgress;
};

XTOOLS_NAMESPACE_END

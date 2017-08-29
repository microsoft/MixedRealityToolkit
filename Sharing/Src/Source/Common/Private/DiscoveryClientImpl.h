//////////////////////////////////////////////////////////////////////////
// DiscoveryClientImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SystemDescription.h"

XTOOLS_NAMESPACE_BEGIN

class DiscoveryClientImpl : public DiscoveryClient
{
public:
	DiscoveryClientImpl();

	/// Broadcast on the local network to discover any remote XTools machines
	virtual void Ping() XTOVERRIDE;

	/// Returns the number of systems discovered
	virtual uint32 GetDiscoveredCount() const XTOVERRIDE;

	/// Returns the information about a discovered system
	virtual DiscoveredSystemPtr GetDiscoveredSystem(uint32 index) const XTOVERRIDE;

	virtual void Update() XTOVERRIDE;

	virtual void AddListener(DiscoveryClientListener* newListener) XTOVERRIDE;

	virtual void RemoveListener(DiscoveryClientListener* oldListener) XTOVERRIDE;

private:
#if RAKPEER_USER_THREADED==1
	RakNet::BitStream m_updateBitStream;
#endif
	
	struct RemoteClient
	{
		DiscoveredSystemPtr								m_remoteSystem;
		RakNet::TimeMS									m_pingTime;
		bool											m_bReceivedResponse;
	};

	typedef ListenerList<DiscoveryClientListener> ListenerList;
	DECLARE_PTR(ListenerList);

	std::map<std::string, RemoteClient>			m_remoteSystems;
	ListenerListPtr								m_listenerList;
	PeerPtr m_peer;
	std::chrono::high_resolution_clock::time_point m_lastPingTime;
	bool										m_bClearedOutStaleClients;
};

DECLARE_PTR(DiscoveryClientImpl)

XTOOLS_NAMESPACE_END

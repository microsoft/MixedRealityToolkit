//////////////////////////////////////////////////////////////////////////
// ProfilerStreamImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfilerStreamImpl : public ProfilerStream, public NetworkConnectionListener
{
public:
	ProfilerStreamImpl(std::string remoteSystemName, uint16 remotePort, const XSocketManagerPtr& socketMgr);

	virtual void AddListener(ProfilerStreamListener* newListener) XTOVERRIDE;

	virtual void RemoveListener(ProfilerStreamListener* oldListener) XTOVERRIDE;

	virtual bool IsConnected() const XTOVERRIDE;

	virtual void Connect() XTOVERRIDE;

	virtual void Disconnect() XTOVERRIDE;

	virtual std::string GetRemoteSystemName() const XTOVERRIDE;

private:
	// NetworkConnectionListener Functions:
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection)  XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	typedef ListenerList<ProfilerStreamListener> ListenerList;
	DECLARE_PTR(ListenerList);

	ListenerListPtr				m_listenerList;

	XSocketManagerPtr			m_socketMgr;
	NetworkConnectionPtr		m_connection;

	std::string					m_remoteSystemName;
	uint16						m_remotePort;
};

XTOOLS_NAMESPACE_END

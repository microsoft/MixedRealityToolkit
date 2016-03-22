//////////////////////////////////////////////////////////////////////////
// ServerRoomManager.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "ServerRoom.h"
#include <Public/SyncManager.h>

XTOOLS_NAMESPACE_BEGIN

class ServerRoomManager : 
	public AtomicRefCounted, 
	public NetworkConnectionListener,
	public ObjectElementListener,
	public RoomEmptyListener
{
public:
	explicit ServerRoomManager(const Sync::SyncManagerPtr& syncMgr);

	void AddConnection(const NetworkConnectionPtr& connection);
	void RemoveConnection(const NetworkConnectionPtr& connection);
	void OnUserDisconnected(UserID userID);

private:
	// NetworkConnectionListener Functions:
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	// ObjectElementListener Functions: 
	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

	// RoomEmptyListener Functions:
	virtual void OnRoomEmpty(const ServerRoomPtr& room) XTOVERRIDE;

	void OnUploadRequest(const NetworkConnectionPtr& connection, NetworkInMessage& message);
	void OnDownloadRequest(const NetworkConnectionPtr& connection, NetworkInMessage& message);

	void SendUploadResponse(const NetworkConnectionPtr& connection, bool bSucceeded, const std::string& failureReason = std::string());
	void SendDownloadResponse(const NetworkConnectionPtr& connection, bool bSucceeded, const BufferPtr& buffer, const std::string& failureReason = std::string());

	ObjectElementPtr m_element;

	std::map<ConnectionGUID, NetworkConnectionPtr> m_connections;
	std::map<RoomID, ServerRoomPtr> m_rooms;

};

DECLARE_PTR(ServerRoomManager)

XTOOLS_NAMESPACE_END

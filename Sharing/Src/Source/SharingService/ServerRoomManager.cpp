//////////////////////////////////////////////////////////////////////////
// ServerRoomManager.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerRoomManager.h"
#include <Private/RoomMessageID.h>

XTOOLS_NAMESPACE_BEGIN

ServerRoomManager::ServerRoomManager(const Sync::SyncManagerPtr& syncMgr)
{
	XStringPtr roomMgrElementName = new XString("RoomMgr");

	m_element = ObjectElement::Cast(syncMgr->GetRootObject()->GetElement(roomMgrElementName));
	if (m_element == nullptr)
	{
		m_element = syncMgr->GetRootObject()->CreateObjectElement(roomMgrElementName, new XString("RoomManager"));
	}

	XTASSERT(m_element);

	m_element->AddListener(this);
}


void ServerRoomManager::AddConnection(const NetworkConnectionPtr& connection)
{
	connection->AddListener(MessageID::RoomAnchor, this);
	m_connections[connection->GetConnectionGUID()] = connection;
}


void ServerRoomManager::RemoveConnection(const NetworkConnectionPtr& connection)
{
	auto mapIter = m_connections.find(connection->GetConnectionGUID());
	if (mapIter != m_connections.end())
	{
		mapIter->second->RemoveListener(MessageID::RoomAnchor, this);
		m_connections.erase(mapIter);
	}
}


void ServerRoomManager::OnUserDisconnected(UserID userID)
{
	// Find the room that this user is in and remove them from it, and close the room if necessary
	std::list<RoomID> deleteRoomList;

	// First remove the user from all rooms, and make a note of which rooms should be deleted
	for (auto roomItr = m_rooms.begin(); roomItr != m_rooms.end(); ++roomItr)
	{
		ServerRoomPtr room = roomItr->second;
		if (room->RemoveUser(userID))
		{
			if (room->IsEmpty() && !room->GetKeepOpen())
			{
				deleteRoomList.push_back(room->GetID());
			}
		}
	}

	// Now go through and delete the empty rooms
	for (auto itr = deleteRoomList.begin(); itr != deleteRoomList.end(); ++itr)
	{
		auto roomItr = m_rooms.find(*itr);
		if (roomItr != m_rooms.end())
		{
			m_rooms.erase(roomItr);
		}
	}
}


void ServerRoomManager::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	RoomMessageID messageType = (RoomMessageID)message.ReadByte();
	switch (messageType)
	{
	case RoomMessageID::AnchorDownloadRequest:
		OnDownloadRequest(connection, message);
		break;

	case RoomMessageID::AnchorUploadRequest:
		OnUploadRequest(connection, message);
		break;
	}
}


void ServerRoomManager::OnElementAdded(const ElementPtr& element)
{
	ServerRoomPtr newRoom = new ServerRoom(this);
	newRoom->BindRemote(element);

	m_rooms[newRoom->GetID()] = newRoom;
}


void ServerRoomManager::OnElementDeleted(const ElementPtr& element)
{
	for (auto itr = m_rooms.begin(); itr != m_rooms.end(); ++itr)
	{
		if (itr->second->GetGUID() == element->GetGUID())
		{
			m_rooms.erase(itr);
			break;
		}
	}
}


void ServerRoomManager::OnRoomEmpty(const ServerRoomPtr& room)
{
	// If the empty room is not set to be kept open, then close it
	if (!room->GetKeepOpen())
	{
		auto roomItr = m_rooms.find(room->GetID());
		if (roomItr != m_rooms.end())
		{
			m_rooms.erase(roomItr);
		}
	}
}


void ServerRoomManager::OnUploadRequest(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	// Read the ID of the room the anchor is in
	RoomID roomID = message.ReadInt64();

	// Find the room
	auto roomItr = m_rooms.find(roomID);
	if (roomItr == m_rooms.end())
	{
		// The target room does not exist; send failure response
		SendUploadResponse(connection, false, "Room not found");
		return;
	}

	ServerRoomPtr room = roomItr->second;
	
	// Read the name of the anchor
	XStringPtr anchorName = message.ReadString();

	// Read the size of the anchor data
	int32 dataSize = message.ReadInt32();

	BufferPtr buffer(new Buffer(dataSize));
	buffer->Resize(dataSize);

	message.ReadArray(buffer->GetData(), dataSize);

	room->SetAnchor(anchorName->GetString(), buffer);

	// Send a response to the sender that the upload was successful
	SendUploadResponse(connection, true);

	// Send a message to all other connections to let them know the anchor has changed
	{
		NetworkOutMessagePtr outMsg = connection->CreateMessage(MessageID::RoomAnchor);
		outMsg->Write(RoomMessageID::AnchorsChangedNotification);
		outMsg->Write(roomID);

		for (auto itr = m_connections.begin(); itr != m_connections.end(); ++itr)
		{
			if (itr->first != connection->GetConnectionGUID() && itr->second->IsConnected())
			{
				itr->second->Send(outMsg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::RoomAnchorChannel, false);
			}
		}

		connection->ReturnMessage(outMsg);
	}
}


void ServerRoomManager::OnDownloadRequest(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	// Read the ID of the room the anchor is in
	RoomID roomID = message.ReadInt64();

	// Find the room
	auto roomItr = m_rooms.find(roomID);
	if (roomItr == m_rooms.end())
	{
		// The target room does not exist; send failure response
		SendDownloadResponse(connection, false, nullptr, "Room not found");
		return;
	}

	ServerRoomPtr room = roomItr->second;

	// Read the name of the anchor
	XStringPtr anchorName = message.ReadString();

	BufferPtr anchorBuffer = room->GetAnchorData(anchorName->GetString());
	if (anchorBuffer == nullptr)
	{
		SendDownloadResponse(connection, false, nullptr, "Anchor name not found");
		return;
	}

	SendDownloadResponse(connection, true, anchorBuffer);
}


void ServerRoomManager::SendUploadResponse(const NetworkConnectionPtr& connection, bool bSucceeded, const std::string& failureReason)
{
	NetworkOutMessagePtr outMsg = connection->CreateMessage(MessageID::RoomAnchor);

	outMsg->Write((byte)RoomMessageID::AnchorUploadResponse);
	
	outMsg->Write((byte)bSucceeded);

	if (!bSucceeded)
	{
		outMsg->Write(failureReason);
	}

	connection->Send(outMsg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::RoomAnchorChannel, true);
}


void ServerRoomManager::SendDownloadResponse(const NetworkConnectionPtr& connection, bool bSucceeded, const BufferPtr& buffer, const std::string& failureReason)
{
	NetworkOutMessagePtr outMsg = connection->CreateMessage(MessageID::RoomAnchor);

	outMsg->Write((byte)RoomMessageID::AnchorDownloadResponse);

	outMsg->Write((byte)bSucceeded);

	if (bSucceeded)
	{
		outMsg->Write((int32)buffer->GetSize());
		outMsg->WriteArray(buffer->GetData(), buffer->GetSize());
	}
	else
	{
		outMsg->Write(failureReason);
	}

	connection->Send(outMsg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::RoomAnchorChannel, true);
}

XTOOLS_NAMESPACE_END

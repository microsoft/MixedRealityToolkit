//////////////////////////////////////////////////////////////////////////
// PacketWrapper.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Helper class to ensure that the packet gets returned to RakNet after use
class PacketWrapper
{
public:
	PacketWrapper(const PeerPtr& peer, RakNet::Packet* packet)
		: m_peer(peer)
		, m_packet(packet)
	{}

	~PacketWrapper() { ReleasePacket(); }

	RakNet::Packet* operator->() { return m_packet; }

	RakNet::Packet* get() { return m_packet; }

	PacketWrapper& operator=(RakNet::Packet* newPacket)
	{
		ReleasePacket();
		m_packet = newPacket;
		return *this;
	}

	bool IsValid() const
	{
		return (m_packet != NULL);
	}

private:
	void ReleasePacket()
	{
		if (m_packet && m_peer)
		{
			m_peer->DeallocatePacket(m_packet);
		}
	}

	PeerPtr			m_peer;
	RakNet::Packet* m_packet;
};

XTOOLS_NAMESPACE_END

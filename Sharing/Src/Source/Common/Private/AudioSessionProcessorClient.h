//////////////////////////////////////////////////////////////////////////
// AudioSessionProcessorClient.h
//
// Listens for broadcast messages then forwards them to all the other connections
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <Public/AudioSessionProcessor.h>

XTOOLS_NAMESPACE_BEGIN


class AudioSessionProcessorClient : public AudioSessionProcessor
{
public:
	AudioSessionProcessorClient();

	virtual void AddConnection(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void RemoveConnection(const NetworkConnectionPtr& connection) XTOVERRIDE;

private:
	AudioPacketPoolPtr	m_audioPacketPool;
};

DECLARE_PTR(AudioSessionProcessorClient)

XTOOLS_NAMESPACE_END

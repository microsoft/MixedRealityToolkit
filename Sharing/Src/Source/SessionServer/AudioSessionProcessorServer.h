// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AudioSessionProcessorServer.h
// Listens for broadcast messages then forwards them to all the other connections
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <Public/AudioSessionProcessor.h>
#include "AudioRouterWithMixer.h"
#include <Private\SpeexProcessor.h>

XTOOLS_NAMESPACE_BEGIN


class AudioSessionProcessorServer : public AudioSessionProcessor
{
public:
	AudioSessionProcessorServer();

	virtual void AddConnection(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void RemoveConnection(const NetworkConnectionPtr& connection) XTOVERRIDE;

private:
	AudioRouterWithMixerPtr     m_routerWithMixer;
	AudioPacketPoolPtr			m_audioPacketPool;
	//SpeexProcessorPtr			m_compressor;
	//SpeexProcessorPtr			m_decompressor;
};

DECLARE_PTR(AudioSessionProcessorServer)

XTOOLS_NAMESPACE_END

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AudioManagerImpl.h
// Implements functionality exposed by AudioManager interface.
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Private/AudioManagerImpl.h>
#include <Private/AudioSessionProcessorClient.h>

XTOOLS_NAMESPACE_BEGIN

AudioManagerImpl::AudioManagerImpl(const NetworkConnectionPtr& connection)
: m_sessionProcessor(new AudioSessionProcessorClient())
, m_bIsMicEnabled(true)		// default to always talking
{
	m_sessionProcessor->AddConnection(connection);
}
 
void AudioManagerImpl::SetMicrophoneEnabled(bool bEnabled)
{
	// TODO: Implement Me!
	m_bIsMicEnabled = bEnabled;


}

XTOOLS_NAMESPACE_END

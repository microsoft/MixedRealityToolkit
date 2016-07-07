// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AudioManagerImpl.h
// Implements functionality exposed by AudioManager interface.
//////////////////////////////////////////////////////////////////////////

#pragma once



XTOOLS_NAMESPACE_BEGIN

class AudioManagerImpl : public AudioManager
{
public:
	AudioManagerImpl(const NetworkConnectionPtr& connection);

	virtual void SetMicrophoneEnabled(bool bEnabled) XTOVERRIDE;

private:
	AudioSessionProcessorPtr	m_sessionProcessor;

	

	bool						m_bIsMicEnabled;
};

DECLARE_PTR(AudioManagerImpl)

XTOOLS_NAMESPACE_END

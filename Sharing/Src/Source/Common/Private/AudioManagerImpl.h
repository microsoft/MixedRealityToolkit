//////////////////////////////////////////////////////////////////////////
// AudioManagerImpl.h
//
// Implements functionality exposed by AudioManager interface.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
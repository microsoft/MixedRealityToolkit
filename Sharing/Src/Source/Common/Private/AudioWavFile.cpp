//////////////////////////////////////////////////////////////////////////
// AudiWavFile.h
//
// Writes a WAV file
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "AudioWavFile.h"

XTOOLS_NAMESPACE_BEGIN


AudioWavFile::AudioWavFile(string filePath)
: m_filePath(filePath)
, m_sampleRate(0)
, m_channelCount(0)
, m_samplesWritten(0)
{
}


void AudioWavFile::WriteHeader(uint32 sampleRate, uint16 channelCount)
{
	m_sampleRate = sampleRate;
	m_channelCount = channelCount;

	m_file.open(m_filePath, ios::binary | ios::out);

	m_file.write("RIFF", 4);
	WriteUInt32(0);
	m_file.write("WAVE", 4);

	m_file.write("fmt ", 4);
	WriteUInt32(16);	                             //fmt chunk size
	WriteUInt16(1);		                             //compression code
	WriteUInt16(m_channelCount);		             //# channels
	WriteUInt32(m_sampleRate);	                     //sample rate;
	WriteUInt32(m_sampleRate * m_channelCount * sizeof(uint16));
	WriteUInt16(m_channelCount * sizeof(uint16));	 // block align
	WriteUInt16(16);	                             // significant bits per sample

	m_file.write("data", 4);
	m_dataLengthPos = m_file.tellp();
	WriteUInt32(0);									 //don't know length yet.

	m_samplesWritten = 0;
}


void AudioWavFile::WriteSamples(const float * samples, uint32 sampleCount)
{
	m_samplesWritten += sampleCount;

	while (sampleCount--)
	{
		int16 intSample = (int16)(*samples++ * 32768.0f);
		WriteUInt16((uint16)intSample);
	}
}

void AudioWavFile::WriteFinalize(void)
{
	streampos fileSize = m_file.tellp();
	m_file.seekp(4);
	WriteUInt32((uint32)fileSize - 8);
	m_file.seekp(m_dataLengthPos);
	WriteUInt32(m_samplesWritten * sizeof(int16));
	m_file.close();
}



void AudioWavFile::WriteUInt16(uint16 val)
{
	char buff[2];

	buff[0] = val & 0xff;
	buff[1] = (val >> 8) & 0xff;

	m_file.write(buff, 2);
}


void AudioWavFile::WriteUInt32(uint32 val)
{
	char buff[4];

	buff[0] = val & 0xff;
	buff[1] = (val >> 8) & 0xff;
	buff[2] = (val >> 16) & 0xff;
	buff[3] = (val >> 24) & 0xff;
	m_file.write(buff, 4);
}


XTOOLS_NAMESPACE_END


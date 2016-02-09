//////////////////////////////////////////////////////////////////////////
// AudiWavFile.h
//
// Writes a WAV file
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <string>

using namespace std;

XTOOLS_NAMESPACE_BEGIN

class AudioWavFile : public RefCounted {


public:
	AudioWavFile(string filePath);

	void WriteHeader(uint32 sampleRate, uint16 channelCount);
	void WriteSamples(const float * samples, uint32 sampleCount);
	void WriteFinalize(void);	// after all samples are written

private:
	void WriteUInt16(uint16 val);
	void WriteUInt32(uint32 val);


	string    m_filePath;
	uint32    m_sampleRate;
	uint16    m_channelCount;
	uint32    m_samplesWritten;
	fstream   m_file;
	streampos m_dataLengthPos;

};

DECLARE_PTR(AudioWavFile)

XTOOLS_NAMESPACE_END

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SpeexCompressProcessor.h
// Converts audio packets from one sample rate to another.
//////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(XTOOLS_PLATFORM_OSX)

#include <Private/AudioProcessor.h>
#include <speex/speex.h>
#include <speex/speex_resampler.h>

XTOOLS_NAMESPACE_BEGIN

class SpeexProcessor : public AudioProcessor
{
	public:
		SpeexProcessor();
		~SpeexProcessor();

		virtual void ProcessAudioPacket(AudioConnectionPtr& audioConnection, AudioPacketPtr& audioPacket);

	private:
		static int CompressVoiceBuffer(float * samples, uint32 numSamples, char * compressedBuffer, uint32 compressedBufferSizeInBytes, SpeexBits *bits, void *enc_state, SpeexResamplerState *resampler48to16);

		SpeexBits encbits;
		void *enc_state;
		SpeexResamplerState *resampler48to16;

		// DELETE ME: decompression for testing
		static void DecompressVoiceBuffer(float * samples, uint32 numSamples, char * compressedBuffer, uint32 nbBytes, SpeexBits *decbits, void *dec_state, SpeexResamplerState *resampler16to48);
		SpeexResamplerState *resampler16to48;
		SpeexBits decbits;
		void *dec_state;
};

DECLARE_PTR(SpeexProcessor)

XTOOLS_NAMESPACE_END

#endif //!defined(XTOOLS_PLATFORM_OSX)

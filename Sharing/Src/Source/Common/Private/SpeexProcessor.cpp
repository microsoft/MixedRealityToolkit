#include "stdafx.h"
#include "SpeexProcessor.h"

#if !defined(XTOOLS_PLATFORM_OSX)

XTOOLS_NAMESPACE_BEGIN

SpeexProcessor::SpeexProcessor()
{
	// TODO - at creation time specify if it will encode or decode. Only initialize the parts needed for that, and do the right thing in ProcessAudioPacket

	// initialize speex encoder
	speex_bits_init(&encbits);
	enc_state = speex_encoder_init(&speex_wb_mode); // wideband 16kHz (speex_nb_mode is narrowband 8kHz, speex_uwb_mode is ultrawideband 32kHz)

	// check frame size
	int frame_size;
	speex_encoder_ctl(enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
	XTASSERT(frame_size == 320);

	// set compression quality/space
	int quality = 8; // 0 is worst (10 bytes per 20ms) to 10 is best (106 bytes per 20ms). Default is 8  - 5 has some noise @ 42 bytes, 8 sounds good @ 70 bytes
	speex_encoder_ctl(enc_state, SPEEX_SET_QUALITY, &quality);

	// set CPU complexity for encoding
	// From docs: the noise level at complexity 1 is between 1 and 2 dB higher than at complexity 10, 
	//			  but the CPU requirements for complexity 10 is about 5 times higher than for complexity 1
	int complexity = 2; // 0-10, lower complexity requires less computation. Default is 2
	speex_encoder_ctl(enc_state, SPEEX_SET_COMPLEXITY, &complexity);  

	// resampler to convert between 48000Hz and 16000Hz
	int err;
	int rawSampleRate = 48000; // sample rate for incoming raw data
	int wideBandSampleRate = 16000; // sample rate expected for speex encoding - must be 16000 for speex_wb_mode
	int resampleQuality = 10; // 0 is worst, 10 is best - higher quality requires more computation
	resampler48to16 = speex_resampler_init(1, rawSampleRate, wideBandSampleRate, resampleQuality, &err);
	
	// DELETE ME - decompress code initialize speex decoder and upsampling just for testing
	speex_bits_init(&decbits);
	dec_state = speex_decoder_init(&speex_wb_mode);
	resampler16to48 = speex_resampler_init(1, wideBandSampleRate, rawSampleRate, resampleQuality, &err);
}

SpeexProcessor::~SpeexProcessor()
{
	speex_bits_destroy(&encbits);
	speex_encoder_destroy(enc_state);
	speex_resampler_destroy(resampler48to16);

	// DELETE ME - decompress code initialize speex decoded and upsampling just for testing
	speex_bits_destroy(&decbits);
	speex_decoder_destroy(dec_state);
	speex_resampler_destroy(resampler16to48);
}

// Takes a 20ms @ 48kHz buffer of 960 float sampless (+/- 1.0) and compresses down to small byte buffer <= 106 bytes depending on quality setting
//   requires pointers to speex state
int SpeexProcessor::CompressVoiceBuffer(float * samplesSrc, uint32 numSamplesSrc, char * compressedBufferDest, uint32 compressedBufferSizeInBytes, SpeexBits *bits, void *enc_state, SpeexResamplerState *resampler48to16)
{
	XTASSERT(compressedBufferSizeInBytes >= 70); // compressed size of 16000Hz data at quality 10 setting
	XTASSERT(numSamplesSrc == 960); // speex expects packets of 20ms, which at 48000 is 960 samples: 0.020 * 48000 = 960
	const int numSamplesAt16000 = 960 / (48000 / 16000); // numSamples downsampled from 48000 to 16000 Hz
	float raw16000Buffer[numSamplesAt16000];

	// first scale all the raw float samples from +/- 1.0 to the +/- 32768.0 that speex expects
	for (unsigned int ii = 0; ii < numSamplesSrc; ++ii)
	{
		samplesSrc[ii] *= 32768.0f;
	}

	// then resample from 48000Hz down to the 16000Hz that speex expects for wideband voice compression
	uint32 in_length = numSamplesSrc;
	uint32 out_length = numSamplesAt16000;
	int err = speex_resampler_process_float(resampler48to16, 0, samplesSrc, &in_length, raw16000Buffer, &out_length);
	err;

	// finally compress raw floats down to small number of bytes
	speex_bits_reset(bits);
	speex_encode(enc_state, raw16000Buffer, bits);
	int nbBytes = speex_bits_write(bits, compressedBufferDest, compressedBufferSizeInBytes);
	return nbBytes;
}


// Takes byte buffer of speex-compressed data (<= 106 bytes depending on quality setting) and decodes to 20ms @ 48kHz buffer of 960 float samples (+/- 1.0)
//   requires pointers to speex state
void SpeexProcessor::DecompressVoiceBuffer(float * samplesDest, uint32 numSamplesDest, char * compressedBuffer, uint32 numCompressedBytes, SpeexBits *decbits, void *dec_state, SpeexResamplerState *resampler16to48)
{
	XTASSERT(numCompressedBytes <= 106); // compressed size of 16000Hz data at quality 10 setting, number of bytes should be that or smaller
	XTASSERT(numSamplesDest == 960); // speex expects packets of 20ms, which at 48000 is 960 samples: 0.020 * 48000 = 960
	const int numSamplesAt16000 = 960 / (48000 / 16000); // numSamples downsampled from 48000 to 16000 Hz
	float raw16000Buffer[numSamplesAt16000];

	// decompress bytes up to 16000Hz raw float stream
	speex_bits_read_from(decbits, compressedBuffer, numCompressedBytes);
	speex_decode(dec_state, decbits, raw16000Buffer);

	// resample from 16000Hz up to 48000Hz for playback
	uint32 in_length = numSamplesAt16000;
	uint32 out_length = numSamplesDest;
	uint32 err = speex_resampler_process_float(resampler16to48, 0, raw16000Buffer, &in_length, samplesDest, &out_length);
	err;

	// and finally unscale raw float samples from +/- 32768.0 down to +/- 1.0 for playback
	for (unsigned int ii = 0; ii < numSamplesDest; ++ii)
	{
		samplesDest[ii] /= 32768.0f;
	}
}

void SpeexProcessor::ProcessAudioPacket(AudioConnectionPtr& /*audioConnection*/, AudioPacketPtr& audioPacket)
{
	const unsigned int numSamples = audioPacket->GetSampleCount();
	const unsigned int numStreams = audioPacket->GetStreamCount();
	const unsigned int numChannels = audioPacket->GetChannelCount();

	const int compressedBufferMaxBytes = 70; // compressed size of 16000Hz data at quality 8 setting
	char compressedBuffer[compressedBufferMaxBytes]; 

	for (uint16 streamIndex = 0; streamIndex < numStreams; ++streamIndex)
	{
		for (uint16 channelIndex = 0; channelIndex < numChannels; ++channelIndex)
		{
			float * const samples = audioPacket->GetSamplePointerForStream(streamIndex, channelIndex);

			uint32 numCompressedBytes = CompressVoiceBuffer(samples, numSamples, compressedBuffer, compressedBufferMaxBytes, &encbits, enc_state, resampler48to16);
			// TODO - need to create new, smaller packet, with compressedBuffer of size numCompressedBytes

			// DELETE ME: test code to decompress
			DecompressVoiceBuffer(samples, numSamples, compressedBuffer, numCompressedBytes, &decbits, dec_state, resampler16to48);
		}
	}
}

XTOOLS_NAMESPACE_END

#endif // !defined(XTOOLS_PLATFORM_OSX)
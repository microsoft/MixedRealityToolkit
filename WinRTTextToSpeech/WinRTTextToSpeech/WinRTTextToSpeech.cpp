// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace Windows::Media::SpeechSynthesis;

SpeechSynthesizer synthesizer = nullptr;

EXTERN_C
{
	/// <summary>
	/// Synthesizes the provided text into wave format with a custom voice.
	/// </summary>
	/// <param name="phrase">The phrase to be synthesized.</param>
	/// <param name="voiceName">The name of the voice to use if available</param>
	/// <param name="data">Will receive the audio data in wave format.</param>
	/// <param name="dataLength">The length of the data being returned.</param>
	/// <returns>True if the synthesis is successful, or false.</returns>
	DLLEXPORT bool __stdcall TrySynthesizePhraseWithCustomVoice(
		const char* phrase,
		const char* voiceName,
		uint8_t** data,
		uint32_t& dataLength)
	{
		// Create the synthesizer on first use.
		if (!synthesizer)
		{
			synthesizer = Windows::Media::SpeechSynthesis::SpeechSynthesizer();

			// Confirm one was created successfully.
			if (!synthesizer)
			{
				*data = nullptr;
				dataLength = 0;
				return false;
			}
		}
		
		// Search for the input voice name
		auto voices = synthesizer.AllVoices();
		for (auto it = begin(voices); it != end(voices); ++it) {
			std::string displayName = to_string((*it).DisplayName());
			std::string voiceString = voiceName;

			if (displayName.find(voiceString) != std::string::npos)
			{
				synthesizer.Voice(*it);
				break;
			}
		}

		// Attempt to synthesize the specified phrase.
		SpeechSynthesisStream speechStream = synthesizer.SynthesizeTextToStreamAsync(to_hstring(phrase)).get();
		if (!speechStream)
		{
			*data = nullptr;
			dataLength = 0;
			return false;
		}

		dataLength = (uint32_t)speechStream.Size();
		
		const Windows::Storage::Streams::IInputStream& inputStream = speechStream.GetInputStreamAt(0);
		speechStream.Close();
		if (!inputStream)
		{
			*data = nullptr;
			dataLength = 0;
			return false;
		}

		Windows::Storage::Streams::DataReader reader = Windows::Storage::Streams::DataReader(inputStream);
		if (!reader)
		{
			inputStream.Close();
			*data = nullptr;
			dataLength = 0;
			return false;
		}

		// Read the syntesized data and pass it back to the caller.
		reader.LoadAsync(dataLength).get();
		*data = new uint8_t[dataLength];
		reader.ReadBytes(array_view<uint8_t>(*data, *data + dataLength));

		reader.Close();
		inputStream.Close();
		
		return true;
	}

	/// <summary>
	/// Frees the memory allocated during synthesis.
	/// </summary>
	/// <param name="data">Pointer to the data to be freed.</param>
	DLLEXPORT void __stdcall FreeSynthesizedData(uint8_t* data)
	{
		if (!data) { return; }
		delete[] data;
	}

	/// <summary>
	/// Synthesizes the provided text into wave format using the default voice.
	/// </summary>
	/// <param name="phrase">The phrase to be synthesized.</param>
	/// <param name="data">Will receive the audio data in wave format.</param>
	/// <param name="dataLength">The length of the data being returned.</param>
	/// <returns>True if the synthesis is successful, or false.</returns>
	DLLEXPORT bool __stdcall TrySynthesizePhrase(
		const char* phrase,
		uint8_t** data,
		uint32_t& dataLength)
	{
		return TrySynthesizePhraseWithCustomVoice(phrase, "Default", data, dataLength);
	}
}

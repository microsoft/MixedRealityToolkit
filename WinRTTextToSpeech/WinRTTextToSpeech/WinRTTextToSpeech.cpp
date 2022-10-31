// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace Windows::Media::SpeechSynthesis;

SpeechSynthesizer synthesizer = NULL;

EXTERN_C
{

/// <summary>
/// Synthesizes the provided text into wave format.
/// </summary>
/// <param name="phrase">The phrase to be synthesized.</param>
/// <param name="buffer">The audio data in wave format.</param>
/// <param name="bufferLength">The length of the data being returned.</param>
/// <returns>True of the synthesis is successful, or false.</returns>
DLLEXPORT bool __stdcall TrySynthesizePhrase(
	const char* phrase,
	BYTE** data,
	uint32_t& dataLength)
{
	// Create the synthesizer on first use.
	if (!synthesizer)
	{
		synthesizer = Windows::Media::SpeechSynthesis::SpeechSynthesizer();

		// Confirm one was created successfully.
		if (!synthesizer)
		{
			data = NULL;
			dataLength = 0;
			return false;
		}
	}

	// Attempt to synthesize the specified phrase.
	SpeechSynthesisStream speechStream = synthesizer.SynthesizeTextToStreamAsync(to_hstring(phrase)).get();
	if (!speechStream)
	{
		data = NULL;
		dataLength = 0;
		return false;
	}

	dataLength = (uint32_t)speechStream.Size();
	
	const Windows::Storage::Streams::IInputStream& inputStream = speechStream.GetInputStreamAt(0);
	speechStream.Close();
	if (!inputStream)
	{
		data = NULL;
		dataLength = 0;
		return false;
	}

	Windows::Storage::Streams::DataReader reader = Windows::Storage::Streams::DataReader(inputStream);
	if (!reader)
	{
		inputStream.Close();
		data = NULL;
		dataLength = 0;
		return false;
	}

	// Read the syntesized data and pass it back to the caller.
	reader.LoadAsync(dataLength).get();
	std::vector<uint8_t>* temp = new std::vector<uint8_t>(dataLength);
	array_view<uint8_t>* tempView =new array_view<uint8_t>(*temp);
	reader.ReadBytes(*tempView);
	*data = temp->data();

	reader.Close();
	inputStream.Close();
	
	return true;
}

}
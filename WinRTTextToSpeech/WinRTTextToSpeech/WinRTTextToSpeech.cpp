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
/// 
/// </summary>
/// <param name="phrase"></param>
/// <param name="buffer"></param>
/// <param name="bufferLength"></param>
/// <returns></returns>
DLLEXPORT bool __stdcall TrySynthesize(
	const char* phrase,
	BYTE* buffer,
	uint64_t& bufferLength)
{
	// Create the synthesizer on first use.
	if (synthesizer == NULL)
	{
		synthesizer = Windows::Media::SpeechSynthesis::SpeechSynthesizer();

		// Confirm one was created successfully.
		if (synthesizer == NULL)
		{
			buffer == NULL;
			bufferLength = 0;
			return false;
		}
	}

	// Attempt to synthesize the specified phrase.
	SpeechSynthesisStream speechStream = synthesizer.SynthesizeTextToStreamAsync(to_hstring(phrase)).get();
	if (speechStream == NULL)
	{
		buffer == NULL;
		bufferLength = 0;
		return false;
	}

	bufferLength = speechStream.Size();
	
	const Windows::Storage::Streams::IInputStream& inputStream = speechStream.GetInputStreamAt(0);
	speechStream.Close();

	Windows::Storage::Streams::DataReader reader = Windows::Storage::Streams::DataReader(inputStream);
	reader.LoadAsync(bufferLength).get();
	// todo reader.ReadBytes(buffer);

	reader.Close();
	inputStream.Close();
	
	return true;
}

/// <summary>
/// 
/// </summary>
/// <param name="buffer"></param>
/// <returns></returns>
DLLEXPORT void FreeBuffer(BYTE* buffer)
{
	if (buffer == NULL) { return; }
	free(buffer);
}

}
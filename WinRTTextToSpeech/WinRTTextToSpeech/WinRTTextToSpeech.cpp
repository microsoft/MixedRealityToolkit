// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Storage.Streams.h>

// Define the interface needed to access the byte data from an IBuffer.
struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
{
	virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
};

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
DLLEXPORT bool __stdcall TrySynthesizePhrase(
	const char* phrase,
	BYTE* data,
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

	reader.LoadAsync(dataLength).get();

	Windows::Foundation::MemoryBuffer buffer = reader.ReadBuffer(dataLength).as<Windows::Foundation::MemoryBuffer>();
	// todo: error check?
	Windows::Foundation::IMemoryBufferReference bufferReference = buffer.CreateReference();
	impl::com_ref<IMemoryBufferByteAccess> bufferAccess = bufferReference.as<IMemoryBufferByteAccess>();
	// todo: error check?
	bufferAccess->GetBuffer(&data, &dataLength);

	bufferAccess->Release();
	buffer.Close();
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
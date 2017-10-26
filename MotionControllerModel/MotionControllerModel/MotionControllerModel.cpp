﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// MotionControllerModel.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Foundation;
using namespace Windows::Perception;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Input::Spatial;

extern "C"
{
    EXTERN_C __declspec(dllexport) BOOL TryGetMotionControllerModel(_In_ UINT32 controllerId, _Out_ UINT32* outputSize, _Outptr_result_bytebuffer_(outputSize) BYTE** outputBuffer)
    {
        ComPtr<ISpatialInteractionManagerInterop> spInteractionInterop;
        ComPtr<IInspectable> spInteractionInteropInspectable;

        // Use WRL to get the SpatialInteractionManager from the active window.
        if (SUCCEEDED(RoGetActivationFactory(HStringReference(L"Windows.UI.Input.Spatial.SpatialInteractionManager").Get(), IID_PPV_ARGS(spInteractionInterop.ReleaseAndGetAddressOf()))) &&
            SUCCEEDED(spInteractionInterop->GetForWindow(GetActiveWindow(), IID_PPV_ARGS(spInteractionInteropInspectable.ReleaseAndGetAddressOf()))))
        {
            // Cast the interop SpatialInteractionManager into an actual SpatialInteractionManager.
            auto spatialInteractionManager = safe_cast<SpatialInteractionManager^>(reinterpret_cast<Platform::Object^>(spInteractionInteropInspectable.Get()));

            // Get the current time, in order to create a PerceptionTimestamp. This will be used to get the currently detected spatial sources.
            Windows::Globalization::Calendar^ c = ref new Windows::Globalization::Calendar;
            PerceptionTimestamp^ perceptionTimestamp = PerceptionTimestampHelper::FromHistoricalTargetTime(c->GetDateTime());
            
            // Get the currently detected spatial sources.
            auto sources = spatialInteractionManager->GetDetectedSourcesAtTimestamp(perceptionTimestamp);

            // Iterate through the detected sources and check against the controller ID that was passed in.
            for (unsigned int i = 0; i < sources->Size; i++)
            {
                if (sources->GetAt(i)->Source->Id == controllerId)
                {
                    // Start getting the renderable model stream asynchronously.
                    auto modelOperation = sources->GetAt(i)->Source->Controller->TryGetRenderableModelAsync();

                    if (modelOperation == nullptr)
                    {
                        return FALSE;
                    }

                    // Wait until the model call finishes.
                    while (modelOperation->Status == AsyncStatus::Started)
                    {
                        std::chrono::milliseconds milis(1);
                        std::this_thread::sleep_for(milis);
                    }

                    auto stream = modelOperation->GetResults();

                    // If the model call failed or the resulting stream is empty, return.
                    if (stream == nullptr || stream->Size == 0)
                    {
                        return FALSE;
                    }

                    // Create a buffer from the stream, to read the contents into a byte array for passback to the app.
                    IBuffer^ buffer = ref new Buffer(stream->Size);
                    auto readOperation = stream->ReadAsync(buffer, stream->Size, InputStreamOptions::None);

                    // Wait for the stream to be fully read into the buffer.
                    while (readOperation->Status == AsyncStatus::Started)
                    {
                        std::chrono::milliseconds milis(1);
                        std::this_thread::sleep_for(milis);
                    }

                    // Now, create a DataReader from the buffer, which can then transfer the bytes into a byte array.
                    DataReader^ reader = DataReader::FromBuffer(buffer);
                    BYTE* pixels = new BYTE[buffer->Length];
                    reader->ReadBytes(Platform::ArrayReference<BYTE>(pixels, buffer->Length));

                    *outputBuffer = pixels;
                    *outputSize = buffer->Length;
                    return TRUE;
                }
            }
        }
        return FALSE;
    }
}
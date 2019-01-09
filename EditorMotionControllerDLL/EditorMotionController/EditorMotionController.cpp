// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// EditorMotionController.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Devices::Haptics;
using namespace Windows::Foundation;
using namespace Windows::Perception;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Input::Spatial;

// This value is standardized according to www.usb.org/developers/hidpage/HUTRR63b_-_Haptics_Page_Redline.pdf
const unsigned short CONTINUOUS_BUZZ_WAVEFORM = 0x1004;
// One second, in 100-nanoseconds.
const long ONE_SECOND = 10000000;

EXTERN_C SpatialInteractionSource^ GetSpatialInteractionSource(UINT32 controllerId)
{
    ComPtr<ISpatialInteractionManagerInterop> spInteractionInterop;
    ComPtr<IInspectable> spInteractionInteropInspectable;

    // Use WRL to get the SpatialInteractionManager from the active window.
    if (SUCCEEDED(RoGetActivationFactory(HStringReference(L"Windows.UI.Input.Spatial.SpatialInteractionManager").Get(), IID_PPV_ARGS(spInteractionInterop.ReleaseAndGetAddressOf()))) &&
        SUCCEEDED(spInteractionInterop->GetForWindow(FindWindow(L"UnityHoloInEditorWndClass", NULL), IID_PPV_ARGS(spInteractionInteropInspectable.ReleaseAndGetAddressOf()))))
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
                return sources->GetAt(i)->Source;
            }
        }
    }
    return nullptr;
}

EXTERN_C __declspec(dllexport) BOOL TryGetMotionControllerModel(_In_ UINT32 controllerId, _Out_ UINT32& outputSize, _Outptr_result_bytebuffer_(outputSize) BYTE*& outputBuffer)
{
    SpatialInteractionSource^ source = GetSpatialInteractionSource(controllerId);

    if (source != nullptr)
    {
        // Start getting the renderable model stream asynchronously.
        task<IRandomAccessStreamWithContentType^> modelOperation;
        try
        {
            modelOperation = create_task(source->Controller->TryGetRenderableModelAsync());
        }
        catch (...)
        {
            return FALSE;
        }

        task_status status = modelOperation.wait();

        if (status != task_status::completed)
        {
            return FALSE;
        }

        IRandomAccessStreamWithContentType^ stream = modelOperation.get();

        // If the model call failed or the resulting stream is empty, return.
        if (stream == nullptr || stream->Size == 0)
        {
            return FALSE;
        }

        // Create a buffer from the stream, to read the contents into a byte array to pass back to the app.
        unsigned int streamSize = safe_cast<unsigned int>(stream->Size);
        IBuffer^ buffer = ref new Buffer(streamSize);
        auto readOperation = create_task(stream->ReadAsync(buffer, streamSize, InputStreamOptions::None));

        status = readOperation.wait();

        if (status != task_status::completed)
        {
            return FALSE;
        }

        outputSize = buffer->Length;

        // Now, create a DataReader from the buffer, which can then transfer the bytes into a byte array.
        DataReader^ reader = DataReader::FromBuffer(buffer);
        outputBuffer = new BYTE[outputSize];
        reader->ReadBytes(Platform::ArrayReference<BYTE>(outputBuffer, outputSize));

        return TRUE;
    }

    return FALSE;
}

EXTERN_C __declspec(dllexport) BOOL StartHaptics(_In_ UINT32 controllerId, _In_ FLOAT intensity, _In_ FLOAT durationInSeconds)
{
    SpatialInteractionSource^ source = GetSpatialInteractionSource(controllerId);

    if (source != nullptr)
    {
        SimpleHapticsController ^simpleHapticsController = source->Controller->SimpleHapticsController;

        for (unsigned int i = 0; i < simpleHapticsController->SupportedFeedback->Size; i++)
        {
            SimpleHapticsControllerFeedback ^simpleHapticsControllerFeedback = simpleHapticsController->SupportedFeedback->GetAt(i);
            if (simpleHapticsControllerFeedback->Waveform.Equals(CONTINUOUS_BUZZ_WAVEFORM))
            {
                if (durationInSeconds.Equals(FLT_MAX))
                {
                    simpleHapticsController->SendHapticFeedback(simpleHapticsControllerFeedback, intensity);
                    return TRUE;
                }
                else
                {
                    Windows::Foundation::TimeSpan ts;
                    ts.Duration = safe_cast<long>(durationInSeconds * ONE_SECOND);

                    simpleHapticsController->SendHapticFeedbackForDuration(simpleHapticsControllerFeedback, intensity, ts);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

EXTERN_C __declspec(dllexport) BOOL StopHaptics(_In_ UINT32 controllerId)
{
    SpatialInteractionSource^ source = GetSpatialInteractionSource(controllerId);

    if (source != nullptr)
    {
        SimpleHapticsController ^simpleHapticsController = source->Controller->SimpleHapticsController;
        simpleHapticsController->StopFeedback();
        return TRUE;
    }

    return FALSE;
}
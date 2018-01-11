////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <SpatialInputUtilities\ControllerRendering.h>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include <Pbr\PbrModel.h>

namespace ControllerRenderSample
{
    // Sampler rendering component for rendering spatial interaction sources.
    class ControllerRenderer
    {
    public:
        ControllerRenderer(
            std::shared_ptr<DX::DeviceResources> deviceResources,
            std::shared_ptr<Pbr::Resources> pbrResources,
            winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& interactionManager);
        ~ControllerRenderer();

        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();

        void Render(
            winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem,
            winrt::Windows::Perception::PerceptionTimestamp const& timestamp);

    private:
        void OnSourceDetected(
            winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
            winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

        winrt::Windows::Foundation::IAsyncAction CacheSpatialInputModelAsync(
            winrt::Windows::UI::Input::Spatial::SpatialInteractionSource const& source);

    private:
        // Cached pointer to device and PBR resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;
        std::shared_ptr<Pbr::Resources> m_pbrResources;

        winrt::Windows::UI::Input::Spatial::SpatialInteractionManager m_interactionManager;
        winrt::event_token m_sourceDetectedEventToken;

        std::map<int, std::shared_ptr<Pbr::Model>> m_spatialInputModels;
        ControllerRendering::ControllerModelCache m_controllerModelCache;
    };
}


////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include <Pbr\PbrModel.h>

namespace ControllerRenderSample
{
    // This sample renderer instantiates a basic rendering pipeline.
    class SpinningCubeRenderer
    {
    public:
        SpinningCubeRenderer(
            std::shared_ptr<DX::DeviceResources> const& deviceResources,
            std::shared_ptr<Pbr::Resources> const& pbrResources);
        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void Render();

        // Property accessors.
        void SetPosition(winrt::Windows::Foundation::Numerics::float3 const& pos) { m_position = pos;  }
        winrt::Windows::Foundation::Numerics::float3 const& GetPosition() { return m_position; }
        void SetOrientation(winrt::Windows::Foundation::Numerics::quaternion const& orientation) { m_orientation = orientation; }
        winrt::Windows::Foundation::Numerics::quaternion const& GetOrientation() { return m_orientation; }

    private:
        // Cached pointer to device and PBR resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;
        std::shared_ptr<Pbr::Resources> m_pbrResources;

        std::shared_ptr<Pbr::Model> m_cubeModel;

        // Variables used with the rendering loop.
        bool m_loadingComplete{ false };

        winrt::Windows::Foundation::Numerics::float3 m_position{ 0.f, 0.f, -2.f };
        winrt::Windows::Foundation::Numerics::quaternion m_orientation { winrt::Windows::Foundation::Numerics::quaternion::identity() };
    };
}


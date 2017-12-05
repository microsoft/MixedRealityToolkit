////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Common\StepTimer.h>
#include <Neso\Common\DeviceResources.h>

namespace Neso {
    class Engine;
}

// Updates, renders, and presents holographic content using Direct3D.
namespace DemoRoom
{
    class DemoRoomMain
    {
    public:
        DemoRoomMain();
        ~DemoRoomMain();

        // Sets the holographic space. This is our closest analogue to setting a new window
        // for the app.
        void SetHolographicSpace(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& holographicSpace);

        // Starts the holographic frame and updates the content.
        void Update();

        // Handle saving and loading of app state owned by AppMain.
        void SaveAppState();
        void LoadAppState();

    private:
        std::unique_ptr<Neso::Engine>        m_engine;

        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        // Render loop timer.
        DX::StepTimer                        m_timer;
    };
}


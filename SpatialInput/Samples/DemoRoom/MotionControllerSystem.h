////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\HolographicScene.h>
#include <Neso\Engine\SpatialInteractionSystem.h>

namespace DemoRoom
{
    struct MotionControllerComponent : Neso::Component<MotionControllerComponent>
    {
        bool IsSource(const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& rhs) const;

        bool attachControllerModel{ false };
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceHandedness requestedHandedness{ winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceHandedness::Unspecified };
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSource source{ nullptr };
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceLocation location{ nullptr };
    };

    ////////////////////////////////////////////////////////////////////////////////
    // MotionControllerSystem
    // This system manages events and behaviors specific to Motion Controllers 
    //
    // You can use the MotionControllerComponent::requestedHandedness to automatically update an Entity's Transform based on handedness 
    // i.e. Attach this entity to the Left or Right controller, and the MotionControllerSystem will automatically update the Transform
    //
    // You can also use the MotionControllerComponent::attachControllerModel to automatically attach the correct 3D model to the object
    // so that the virtual controller will be rendered in the same position as the physical controller
    class MotionControllerSystem : 
        public Neso::System<MotionControllerSystem>, 
        public Neso::IPredictionUpdateListener,
        public Neso::ISpatialInteractionListener
    {
    public:
        using System::System;

    protected:
        // System
        void Start() override;
        void Stop() override;

        // IPredictionUpdateListener
        void OnPredictionUpdated(
            Neso::IPredictionUpdateListener::PredictionUpdateReason reason,
            const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordinateSystem,
            const winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction) override;

        // ISpatialInteractionListener
        void OnSourceDetected(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

        void OnSourceUpdated(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

        void OnSourceLost(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

    private:
        void RefreshComponentsForSource(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& source);
    };
}


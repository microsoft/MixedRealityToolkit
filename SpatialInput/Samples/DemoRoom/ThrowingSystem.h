////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "ToolSystem.h"

namespace DemoRoom
{
    struct ThrowingComponent : Neso::Component<ThrowingComponent>
    {
        void SetEnabled(bool enable) override;
        void Destroy() override;

        Neso::SharedEntity ballObject;

        float distanceFromPointer = 0.05f;
        float scale = 0.25f;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // ThrowingInteractionSystem
    // This ToolSystem manages the Throwing tool which allows you to throw baseballs in 3D scene
    class ThrowingInteractionSystem : public ToolSystem<ThrowingInteractionSystem, ThrowingComponent>
    {
    public:
        using ToolSystem::ToolSystem;

    protected:
        // System
        void Update(float dt) override;

        // IInteractionModeSystem
        std::wstring_view GetInstructions() const override;
        std::wstring_view GetDisplayName() const override;
        Neso::SharedEntity CreateToolSelector() const override;

        // ISpatialInteractionListener
        void OnSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

        void OnSourceReleased(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;
    };
}


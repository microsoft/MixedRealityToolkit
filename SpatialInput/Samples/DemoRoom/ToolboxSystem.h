////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include "ToolSystem.h"

namespace DemoRoom
{
    class ToolSystemBase;

    struct ToolComponent : Neso::Component<ToolComponent>
    {
        std::wstring title;
        std::wstring description;
        Neso::detail::type_id toolType{ typeid(nullptr_t) };
    };

    ////////////////////////////////////////////////////////////////////////////////
    // ToolboxSystem
    // This system manages the ToolSystems and manages the two Entities that represent the left and right Motion Controllers
    class ToolboxSystem : 
        public Neso::System<ToolboxSystem>,
        public Neso::ISpatialInteractionListener
    {
    public:
        using System::System;

        void AddToolSystem(std::shared_ptr<ToolSystemBase> system);
        void RemoveToolSystem(const std::shared_ptr<ToolSystemBase>& system);

    protected:
        // System
        void Start() override;
        void Update(float dt) override;
        void Stop() override;

        // ISpatialInteractionListener
        void OnSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) override;

    private:
        Neso::detail::type_map<std::shared_ptr<ToolSystemBase>> m_selectors;
        Neso::detail::type_map<Neso::SharedEntity> m_selectorObjects;

        bool m_showToolbox{ false };

        enum ControllerHand {
            Left, Right, Count
        };

        static std::wstring_view ControllerHandToString(ControllerHand hand);
        static winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceHandedness ControllerHandToHandedness(ControllerHand hand);

        struct ControllerContext {
            Neso::SharedEntity Controller;
            Neso::SharedEntity DebugText;
            ControllerHand Hand;
        };

        void SwitchToolType(Neso::Entity& entity, const Neso::detail::type_id& newType);

        Neso::SharedEntity FindController(const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& source);

        std::array<ControllerContext, ControllerHand::Count> m_controllers;

        Neso::SharedEntity m_instructionalText{ nullptr };
    };
}

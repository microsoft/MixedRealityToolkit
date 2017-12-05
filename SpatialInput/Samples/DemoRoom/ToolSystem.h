////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once
#include <Neso\Engine\Engine.h>
#include <Neso\Engine\EntityStore.h>
#include <Neso\Engine\EasingSystem.h> // Easing
#include <Neso\Engine\CommonComponents.h> // PbrRenderable
#include <Neso\Engine\SpatialInteractionSystem.h> 

namespace DemoRoom
{
    ////////////////////////////////////////////////////////////////////////////////
    // ToolSystemBase
    // Base abstract class for all ToolSystems
    class ToolSystemBase abstract : public Neso::SystemBase
    {
    public:
        using SystemBase::SystemBase;

        virtual std::wstring_view GetInstructions() const = 0;
        virtual std::wstring_view GetDisplayName() const = 0;
        
        virtual Neso::SharedEntity CreateToolSelector() const = 0;

        virtual void Register(std::vector<Neso::SharedEntity> entities) = 0;
        virtual void Unregister() = 0;
        virtual void Activate(Neso::Entity& entity) = 0;
        virtual void Deactivate(Neso::Entity& entity) = 0;
    };

    struct ToolSelectorKey : Neso::Component<ToolSelectorKey>
    {
        Neso::detail::type_id type{ typeid(nullptr_t) };
    };

    struct ToolSelectorPrefab : Neso::EntityPrefab<Neso::Transform, Neso::PbrRenderable, ToolSelectorKey, Neso::RigidBody, Neso::Easing>
    {
        static Neso::ComponentMap Make(Neso::ComponentStore& store);
    };

    // CRTP implementation helper
    // Usage: class MyToolSystem : ToolSystem<MyToolSystem> { /* functions + data members */ };
    // Adds functionality to automatically register to listeners and helpers to access entities 
    // that actually have the associated ToolComponent attached and enabled
    template<typename T, typename ToolComponent>
    class ToolSystem abstract : 
        public ToolSystemBase, 
        public Neso::ISpatialInteractionListener,
        public std::enable_shared_from_this<T>
    {
    public:
        using ToolSystemBase::ToolSystemBase;

        // System
        Neso::detail::type_id type() const override
        {
            return typeid(T);
        }

    protected:
        // System
        void Start() override
        {
            m_engine.Get<ToolboxSystem>()->AddToolSystem(shared_from_this());
        }

        void Stop() override
        {
            m_engine.Get<ToolboxSystem>()->RemoveToolSystem(shared_from_this());
        }

        // ToolSystemBase
        void Register(std::vector<Neso::SharedEntity> entities) override
        {
            m_entities = std::move(entities);

            for (auto& entity : m_entities)
            {
                entity->Add<ToolComponent>()->SetEnabled(false);
            }

            m_engine.Get<SpatialInteractionSystem>()->AddListener(shared_from_this());
        }

        void Unregister() override 
        {
            m_engine.Get<SpatialInteractionSystem>()->RemoveListener(shared_from_this());

            for (auto& entity : m_entities)
            {
                entity->Remove<ToolComponent>();
            }

            m_entities.clear();
        }

        void Activate(Neso::Entity& entity) override
        {
            entity.Get<ToolComponent>()->SetEnabled(true);
        }

        void Deactivate(Neso::Entity& entity) override
        {
            entity.Get<ToolComponent>()->SetEnabled(false);
        }

        // Internal helpers
        std::vector<std::tuple<Neso::Entity*, ToolComponent*>> GetEnabledEntities() const
        {
            std::vector<std::tuple<Neso::Entity*, ToolComponent*>> entities;

            for (auto& entity : m_entities)
            {
                auto comp = entity->Get<ToolComponent>();
                if (comp->IsEnabled())
                {
                    entities.push_back(std::make_tuple(entity.get(), std::move(comp)));
                }
            }

            return entities;
        }

        std::optional<std::tuple<Neso::Entity*, ToolComponent*>> TryGetEntityFromSource(const winrt::Windows::UI::Input::Spatial::SpatialInteractionSource& source) const
        {
            for (auto& entity : m_entities)
            {
                auto comp = entity->Get<ToolComponent>();
                if (comp->IsEnabled())
                {
                    if (entity->Get<MotionControllerComponent>()->IsSource(source))
                    {
                        return std::make_tuple(entity.get(), std::move(comp));
                    }
                }
            }
            
            return std::nullopt;
        }

        std::vector<Neso::SharedEntity> m_entities;
    };
}

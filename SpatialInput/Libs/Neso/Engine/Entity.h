////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\EngineCommon.h>
#include <Neso\Engine\Component.h>

namespace Neso {
    
    class Engine;

    class EntityStore;
    class Entity;
    using SharedEntity = std::shared_ptr<Entity>;

    ////////////////////////////////////////////////////////////////////////////////
    // Entity
    // An Entity is a collection of Components and represents a single object in the world
    // It does not contain any logic, only helper functions to Get/Add/Remove Components from an Entity
    class Entity : public Destroyable, public Enableable, public std::enable_shared_from_this<Entity>
    {
    public:
        using EntityId = uint64_t;

        template<typename T>
        T* Get() {
            return m_components.Get<T>();
        }

        template<typename T>
        void Remove() {
            m_components.Remove<T>();
        }

        template<typename T>
        T* Add() {
            return m_components.Add<T>(m_engine.Get<ComponentStore>()->CreateComponent<T>());
        }

        template<typename... ComponentTs>
        std::tuple<ComponentTs*...> TryGetComponents() {
            return std::make_tuple(m_components.TryGet<ComponentTs>()...);
        }

        Entity(ComponentMap components, EntityId id, Engine& core);
        virtual ~Entity();

        SharedEntity Clone() const;

        EntityId Id() const { return m_id; }

        void Destroy() override;
        void SetEnabled(bool) override;

    private:
        ComponentMap m_components;
        EntityId m_id;
        Engine& m_engine;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // EntityPrefab
    // Instead of deriving from Entity to define new objects we use Prefabs to specify 
    // what components an entity should have, and what the initial values should be

    // Example, an Entity 
    // Declaration: 
    // struct GiantBall : EntityPrefab<Transform, BallComponent> {
    //     static ComponentMap Make(ComponentStore& store) {
    //         auto components = EntityPrefab::Make(store);
    //         components.Get<Transform>()->scale = { 500.0f, 500.0f, 500.0f };
    //         return components;
    //     }
    // };
    
    // Usage: 
    // auto giantBall = m_engine.Get<EntityStore>()->Create<GiantBall>();
    // Now "giantBall->Get<Transform>()->scale" will be set to "{ 500.0f, 500.0f, 500.0f }"
    template<typename... ComponentTs>
    struct EntityPrefab
    {
        static_assert(detail::all_components<ComponentTs...>::value, "All components should derive from Component");

        using Components = std::tuple<ComponentTs...>;

        static ComponentMap Make(ComponentStore& store)
        {
            return store.CreateComponentMap<ComponentTs...>();
        }
    };
}

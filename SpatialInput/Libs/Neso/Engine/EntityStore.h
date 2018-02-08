////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\ComponentStore.h>

namespace Neso
{
    ////////////////////////////////////////////////////////////////////////////////
    // EntityStore
    // Manages all the instances of Entities in the engine. See Entity.h
    // Also allows creation of Entities through the EntityPrefab class
    class EntityStore : public System<EntityStore>
    {
    public:
        using System::System;

        SharedEntity CreateFromComponentMap(ComponentMap components);

        template<typename PrefabT>
        SharedEntity Create()
        {
            static_assert(detail::tuple_all_components<typename PrefabT::Components>::value, "Prefab should have a list of Components");

            return CreateFromComponentMap(PrefabT::Make(*m_engine.Get<ComponentStore>()));
        }

        template<typename... ComponentTs>
        std::vector<std::tuple<Entity*, ComponentTs*...>> GetComponentsWithEntity()
        {
            static_assert(sizeof...(ComponentTs) > 0, "Need at least one component");
            static_assert(detail::all_components<ComponentTs...>::value, "Ts should all derive from Component");

            std::vector<std::tuple<Entity*, ComponentTs*...>> components;

            for (auto& object : m_objects)
            {
                auto requested_components = object->TryGetComponents<ComponentTs...>();

                if (AllValidComponents(requested_components))
                {
                    components.push_back(std::tuple_cat(std::make_tuple(object.get()), std::move(requested_components)));
                }
            }

            return components;
        }

        template<typename... ComponentTs>
        std::vector<std::tuple<ComponentTs*...>> GetComponents()
        {
            static_assert(sizeof...(ComponentTs) > 0, "Need at least one component");
            static_assert(detail::all_components<ComponentTs...>::value, "Ts should all derive from Component");

            std::vector<std::tuple<ComponentTs*...>> components;

            for (auto& object : m_objects)
            {
                auto requested_components = object->TryGetComponents<ComponentTs...>();

                if (AllValidComponents(requested_components))
                {
                    components.push_back(std::move(requested_components));
                }
            }

            return components;
        }

    protected:
        void Update(float) override;

        template<typename Tuple>
        bool AllValidComponents(const Tuple& components)
        {
            bool allValidComponents = true;
            detail::tuple_for_each(components, [&](auto* component) {
                allValidComponents &= component != nullptr && !component->IsDestroyed() && component->IsEnabled();
            });
            return allValidComponents;
        }

    private:
        Entity::EntityId m_nextId = 0;
        std::vector<SharedEntity> m_objects;

        SharedEntity AddEntity(SharedEntity obj);
        Entity::EntityId GetNextId();
    };
}

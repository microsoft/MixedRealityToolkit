#pragma once

#include "EngineTypeTraits.h"
#include "Engine.h"
#include "EngineCommon.h"
#include "Component.h"

#include <functional>

namespace Neso
{
    class ComponentStore : public System<ComponentStore>, public Factory<SharedComponent, std::function<SharedComponent()>>
    {
    public:
        using System::System;

        template<typename... ComponentTs>
        ComponentMap CreateComponentMap()
        {
            static_assert(detail::all_components<ComponentTs...>::value, "Ts should all be a component");

            return ComponentMap{ { typeid(ComponentTs), CreateComponent<ComponentTs>() }... };
        }

        template<typename ComponentT>
        SharedComponent CreateComponent()
        {
            static_assert(detail::is_component<ComponentT>::value, "T should be a component");

            const detail::type_id key(typeid(ComponentT));

            auto it = m_producers.find(key);
            if (it == m_producers.end())
            {
                m_producers.insert(it, { key, [] { return std::make_shared<ComponentT>(); } });
            }

            return CreateComponent(typeid(ComponentT));
        }

        ComponentMap Clone(const ComponentMap& componentMap);

    protected:
        void Update(float) override;

    private:
        detail::unordered_type_map<std::vector<SharedComponent>> m_components;

        SharedComponent CreateComponent(const detail::type_id& typeId);
    };
}
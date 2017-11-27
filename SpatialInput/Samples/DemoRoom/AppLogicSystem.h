#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\EntityStore.h>

namespace DemoRoom 
{
    class AppLogicSystem : public Neso::System<AppLogicSystem>
    {
    public:
        using System::System;

    protected:
        void Update(float /*dt*/) override
        {
            for (auto& componentSet : m_engine.Get<Neso::EntityStore>()->GetComponentsWithEntity<Neso::Transform>())
            {
                auto[entity, transform] = componentSet;

                // Destroy any objects that fall too far away (Baseballs and Bullets)
                if (transform->position.y < -10.0f)
                {
                    entity->Destroy();
                }
            }
        }
    };
}
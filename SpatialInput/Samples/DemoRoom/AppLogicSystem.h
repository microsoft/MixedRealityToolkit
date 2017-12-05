////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\EntityStore.h>

namespace DemoRoom 
{
    ////////////////////////////////////////////////////////////////////////////////
    // AppLogicSystem
    // Simple system to destroy objects that get too far away from the user
    class AppLogicSystem : public Neso::System<AppLogicSystem>
    {
    public:
        using System::System;

    protected:
        void Update(float /*dt*/) override
        {
            for (auto &componentSet : m_engine.Get<Neso::EntityStore>()->GetComponentsWithEntity<Neso::Transform>())
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

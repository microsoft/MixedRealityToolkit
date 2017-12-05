////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "ToolSystem.h"
#include <Neso\Engine\ComponentStore.h>

using namespace Neso;

namespace DemoRoom {

ComponentMap ToolSelectorPrefab::Make(ComponentStore& store)
{
    auto components = EntityPrefab::Make(store);

    components.Get<RigidBody>()->angularVelocity = { 0.0f, -3.0f, 0.0f }; // Spin in place
    components.Get<RigidBody>()->dampingFactor = 1.0f;
    components.Get<Easing>()->PositionEasingFactor = 0.1f;

    return components;
}

} // namespace DemoRoom

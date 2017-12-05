////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PhysicsSystem.h"
#include "EntityStore.h"
#include "CommonComponents.h"

using namespace Neso;
using namespace winrt::Windows::Foundation::Numerics;

const float3 PhysicsSystem::EarthGravity = { 0, -9.8f, 0 };

void PhysicsSystem::Update(float dt)
{
    for (auto[transform, rigidBody] : m_engine.Get<EntityStore>()->GetComponents<Transform, RigidBody>())
    {
        rigidBody->velocity += rigidBody->acceleration * dt;
        transform->position += rigidBody->velocity * dt;

        const float3 adjustedAngular = winrt::Windows::Foundation::Numerics::transform(rigidBody->angularVelocity, inverse(transform->orientation));

        const float angle = length(adjustedAngular);
        if (angle > 0.0f)
        {
            const float3 axis = adjustedAngular / angle;
            transform->orientation *= make_quaternion_from_axis_angle(axis, angle * dt);
        }

        rigidBody->velocity *= rigidBody->dampingFactor;
        rigidBody->angularVelocity *= rigidBody->dampingFactor;
    }
}


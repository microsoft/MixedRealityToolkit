////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "ShootingSystem.h"
#include "ToolboxSystem.h"
#include "EntityPrefabs.h"

#include <Neso\Engine\CommonComponents.h>
#include <SpatialInputUtilities\Haptics.h>

using namespace Neso;
using namespace DemoRoom;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;
using namespace std::chrono_literals;

std::wstring_view ShootingInteractionSystem::GetInstructions() const
{
    return L"Pull the trigger to fire the gun.\n\n"
           L"You can feel controller vibrate for each bullet.\n\n";
}

std::wstring_view ShootingInteractionSystem::GetDisplayName() const
{
    return L"Shooting";
}

SharedEntity ShootingInteractionSystem::CreateToolSelector() const
{
    auto selector = m_engine.Get<EntityStore>()->Create<ToolSelectorPrefab>();

    selector->Get<PbrRenderable>()->ResetModel("Gun");
    selector->Get<ToolSelectorKey>()->type = type();

    return selector;
}

void ShootingInteractionSystem::Register(std::vector<SharedEntity> entities)
{
    ToolSystem::Register(std::move(entities));

    // These values were created through trial and error and would be specific to the particular 3D model you choose to use for your gun.
    // In this scenario, we need to generate two transforms. 
    // First transform is used to align the 3D model with the physical MotionController: modelToController
    // Second transform is used to align the 3D model's barrel with the physical MotionController: barrelToController

    // When using the MotionControllerComponent, the MotionControllerSystem will update the Transform component of the same object to match the controller's location.

    // The "modelToController" is to transform from the object's model space, to the location of the controller
    const float4x4 modelToControllerRotation = make_float4x4_from_yaw_pitch_roll(DirectX::XMConvertToRadians(180), DirectX::XMConvertToRadians(70), 0.0f);
    const float4x4 modelToControllerTranslation = make_float4x4_translation(0, 0.05f, 0.0f);
    const float4x4 modelToController = modelToControllerRotation * modelToControllerTranslation;

    // The "barrelToController" is to transform from the tip of the barrel to the location of the controller
    const float4x4 barrelToController = make_float4x4_translation(0.0f, 0.0675f, -0.22f) * make_float4x4_rotation_x(DirectX::XMConvertToRadians(-70));
    
    for (auto& entity : m_entities) 
    {
        auto gun = m_engine.Get<EntityStore>()->Create<Gun>();
        gun->Get<MotionControllerComponent>()->requestedHandedness = entity->Get<MotionControllerComponent>()->requestedHandedness;
        gun->Get<PbrRenderable>()->Offset = modelToController;

        entity->Get<ShootingComponent>()->barrelToController = barrelToController;
        entity->Get<ShootingComponent>()->gun = std::move(gun);
        entity->Get<ShootingComponent>()->SetEnabled(false);
    }
}

void ShootingInteractionSystem::Activate(Entity& entity)
{
    ToolSystem::Activate(entity);
    entity.Get<PbrRenderable>()->SetEnabled(false);
}

void ShootingInteractionSystem::Deactivate(Entity& entity)
{
    entity.Get<PbrRenderable>()->SetEnabled(true);
    ToolSystem::Deactivate(entity);
}

void ShootingInteractionSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
    if (auto enabledEntity = TryGetEntityFromSource(args.State().Source()))
    {
        auto shooting = std::get<ShootingComponent*>(*enabledEntity);

        if (args.PressKind() == SpatialInteractionPressKind::Select)
        {
            const float4x4 barrelToWorld = shooting->barrelToController * shooting->gun->Get<Transform>()->GetMatrix();

            const float3 position = float4x4_util::position(barrelToWorld);
            const quaternion orientation = make_quaternion_from_rotation_matrix(barrelToWorld);

            const float3 forward = float4x4_util::forward(barrelToWorld);
            const float3 bulletVelocity = forward * shooting->bulletSpeed;

            // Create bullet and send it on it's merry way
            auto bullet = m_engine.Get<EntityStore>()->Create<Bullet>();

            bullet->Get<Transform>()->position = position;
            bullet->Get<Transform>()->orientation = orientation;
            bullet->Get<RigidBody>()->velocity = bulletVelocity;

            SpatialInputUtilities::Haptics::SendContinuousBuzzForDuration(args.State().Source(), 125ms);
        }
    }
}

void ShootingInteractionSystem::OnSourceUpdated(const SpatialInteractionSourceEventArgs& args)
{
    if (auto enabledEntity = TryGetEntityFromSource(args.State().Source()))
    {
        auto[entity, shooting] = *enabledEntity;
        
        // Show the controllers while we're holding grasp, to help show how the model relates to the real world object
        const bool shouldRenderController = args.State().IsGrasped();

        entity->Get<PbrRenderable>()->SetEnabled(shouldRenderController);
        shooting->gun->Get<PbrRenderable>()->AlphaMultiplier = (shouldRenderController) ? std::make_optional(0.25f) : std::nullopt;
    }
}

void ShootingComponent::SetEnabled(bool enable) 
{
    Enableable::SetEnabled(enable);

    if (gun) {
        gun->SetEnabled(enable);
    }
}

void ShootingComponent::Destroy() 
{
    Destroyable::Destroy();

    if (gun) {
        gun->Destroy();
    }
}


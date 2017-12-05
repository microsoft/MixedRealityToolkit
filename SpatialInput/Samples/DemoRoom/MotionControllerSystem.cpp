////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "MotionControllerSystem.h"

#include <Neso\Engine\CommonComponents.h>
#include <Neso\Engine\HolographicRenderer.h>
#include <Neso\Engine\EntityStore.h>
#include <Neso\Engine\PbrModelCache.h>

#include <SpatialInputUtilities\ControllerRendering.h>

#include <Pbr\PbrModel.h>

using namespace Neso;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Input::Spatial;

namespace 
{
    std::string ControllerModelKeyToString(const std::tuple<uint16_t, uint16_t, uint16_t, SpatialInteractionSourceHandedness>& tuple)
    {
        std::stringstream ss;

        ss << "MotionController_"
            << std::get<0>(tuple) << "_"
            << std::get<1>(tuple) << "_"
            << std::get<2>(tuple) << "_"
            << static_cast<uint16_t>(std::get<3>(tuple));

        return ss.str();
    }

    std::future<void> LoadAndCacheModel(
        const SpatialInteractionSource& source, 
        Engine& engine)
    {
        const auto controllerModelName = ControllerModelKeyToString(ControllerRendering::GetControllerModelKey(source));

        auto pbrModelCache = engine.Get<PbrModelCache>();
        if (!pbrModelCache->ModelExists(controllerModelName.c_str()))
        {
            const auto pbrResources = engine.Get<HolographicRenderer>()->GetPbrResources();

            const auto model = co_await ControllerRendering::TryLoadRenderModelAsync(pbrResources, source);

            if (model) 
            {
                pbrModelCache->RegisterModel(controllerModelName, model->Clone(*pbrResources));
            }
            else
            {
                debug_log("Failed to load model for source %d", source.Id());
            }
        }
    }
}

namespace DemoRoom {

void MotionControllerSystem::Start()
{
    m_engine.Get<HolographicScene>()->AddPredictionUpdateListener(shared_from_this());
    m_engine.Get<SpatialInteractionSystem>()->AddListener(shared_from_this());
}

void MotionControllerSystem::OnPredictionUpdated(
    IPredictionUpdateListener::PredictionUpdateReason /*reason*/,
    const SpatialCoordinateSystem& coordinateSystem,
    const HolographicFramePrediction& prediction)
{
    // Update the positions of the controllers based on the current timestamp.
    for (auto& sourceState : m_engine.Get<SpatialInteractionSystem>()->GetInteractionManager().GetDetectedSourcesAtTimestamp(prediction.Timestamp()))
    {
        for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<Transform, MotionControllerComponent>())
        {
            auto[transform, controller] = componentSet;

            RefreshComponentsForSource(sourceState.Source());

            if (controller->IsSource(sourceState.Source()))
            {
                const SpatialInteractionSourceLocation location = sourceState.Properties().TryGetLocation(coordinateSystem);

                controller->location = location;

                if (location)
                {
                    transform->position = location_util::position(location);
                    transform->orientation = location_util::orientation(location);
                }
            }
        }
    }
}

void MotionControllerSystem::Stop()
{
    m_engine.Get<HolographicScene>()->RemovePredictionUpdateListener(shared_from_this());
    m_engine.Get<SpatialInteractionSystem>()->RemoveListener(shared_from_this());
}

void MotionControllerSystem::RefreshComponentsForSource(const SpatialInteractionSource& source)
{
    for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponentsWithEntity<MotionControllerComponent>())
    {
        auto[entity, controller] = componentSet;

        fail_fast_if(controller->requestedHandedness == SpatialInteractionSourceHandedness::Unspecified, "Unspecified is not supported yet");

        if (controller->source == nullptr && source.Handedness() == controller->requestedHandedness)
        {
            controller->source = source;
            debug_log("Attached source id %d to entity %lld with requested handedness %d", source.Id(), entity->Id(), static_cast<uint32_t>(controller->requestedHandedness));
        }
    }
}

void MotionControllerSystem::OnSourceUpdated(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<PbrRenderable, MotionControllerComponent>())
        {
            auto[pbr, controller] = componentSet;

            if (controller->IsSource(args.State().Source()) && controller->attachControllerModel)
            {
                // If we don't have a model yet, set the ModelName so PbrModelCache will update the model
                if (!pbr->Model)
                {
                    pbr->ResetModel(ControllerModelKeyToString(ControllerRendering::GetControllerModelKey(controller->source)));
                }
                else
                {
                    ControllerRendering::ArticulateControllerModel(ControllerRendering::GetArticulateValues(args.State()), *pbr->Model);
                }
            }
        }
    }
}

void MotionControllerSystem::OnSourceDetected(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        // Attempt to load any controller models into the PbrModelCache
        (void)LoadAndCacheModel(args.State().Source(), m_engine);

        // Update any components with their new Source 
        RefreshComponentsForSource(args.State().Source());
    }
}

void MotionControllerSystem::OnSourceLost(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() == SpatialInteractionSourceKind::Controller)
    {
        for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<MotionControllerComponent>())
        {
            auto[controller] = componentSet;

            if (controller->IsSource(args.State().Source()))
            {
                controller->source = nullptr;
                controller->location = nullptr;
            }
        }
    }
}

bool MotionControllerComponent::IsSource(const SpatialInteractionSource& rhs) const 
{
    return (this->source && rhs) ? this->source.Id() == rhs.Id() : false;
}

} // namespace DemoRoom

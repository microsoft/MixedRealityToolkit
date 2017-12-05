////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "HolographicScene.h"
#include "CommonComponents.h"

using namespace Neso;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;

HolographicScene::HolographicScene(
    Engine& core, 
    winrt::Windows::Graphics::Holographic::HolographicSpace holographicSpace) :
    System(core),
    m_holographicSpace(std::move(holographicSpace))
{}

void HolographicScene::Initialize()
{
    m_stageFrameOfReference = SpatialStageFrameOfReference::Current();

    // Create a fallback frame of reference 1.5 meters under the HMD when we start-up
    m_stationaryFrameOfReference = SpatialLocator::GetDefault().CreateStationaryFrameOfReferenceAtCurrentLocation(
        winrt::Windows::Foundation::Numerics::float3{0.0f, -1.5f, 0.0f});

    m_spatialStageCurrentChanged = SpatialStageFrameOfReference::CurrentChanged(
        std::bind(&HolographicScene::OnCurrentStageChanged, this));
}

void HolographicScene::Update(float)
{
    m_currentFrame = m_holographicSpace.CreateNextFrame();

    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicSpaceCreateNextFrame);
}

void HolographicScene::Uninitialize()
{
    SpatialStageFrameOfReference::CurrentChanged(m_spatialStageCurrentChanged);

    m_currentFrame = nullptr;
    m_stationaryFrameOfReference = nullptr;
    m_stageFrameOfReference = nullptr;
}

void HolographicScene::UpdateCurrentPrediction()
{
    m_currentFrame.UpdateCurrentPrediction();

    OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason::HolographicFrameUpdatePrediction);
}

void HolographicScene::OnCurrentStageChanged()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_stageFrameOfReference = SpatialStageFrameOfReference::Current();
}

void HolographicScene::OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason)
{
    const HolographicFramePrediction prediction = m_currentFrame.CurrentPrediction();
    const SpatialCoordinateSystem coordinateSystem = WorldCoordinateSystem();

    for (const auto& listener : m_predictionUpdatelisteners.PurgeAndGetListeners())
    {
        listener->OnPredictionUpdated(reason, coordinateSystem, prediction);
    }
}

void HolographicScene::AddPredictionUpdateListener(std::shared_ptr<IPredictionUpdateListener> listener)
{
    m_predictionUpdatelisteners.Add(std::move(listener));
}

void HolographicScene::RemovePredictionUpdateListener(std::shared_ptr<IPredictionUpdateListener> listener)
{
    m_predictionUpdatelisteners.Remove(std::move(listener));
}

SpatialCoordinateSystem HolographicScene::WorldCoordinateSystem() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_stageFrameOfReference)
    {
        return m_stageFrameOfReference.CoordinateSystem();
    }
    else
    {
        return m_stationaryFrameOfReference.CoordinateSystem();
    }
}

PerceptionTimestamp HolographicScene::CurrentTimestamp() const
{
    return CurrentFrame().CurrentPrediction().Timestamp();
}

HolographicFrame HolographicScene::CurrentFrame() const
{
    fail_fast_if(m_currentFrame == nullptr);
    return m_currentFrame;
}

HolographicSpace HolographicScene::HolographicSpace() const
{
    fail_fast_if(m_holographicSpace == nullptr);
    return m_holographicSpace;
}


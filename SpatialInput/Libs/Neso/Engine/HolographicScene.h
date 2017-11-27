#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\ListenerCollection.h>

namespace Neso
{
    class IPredictionUpdateListener abstract
    {
    public:
        enum PredictionUpdateReason
        {
            HolographicSpaceCreateNextFrame,
            HolographicFrameUpdatePrediction,
        };

        virtual void OnPredictionUpdated(
            PredictionUpdateReason reason,
            const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordinateSystem,
            const winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction) = 0;
    };

    class HolographicScene : public System<HolographicScene>
    {
    public:
        using System::System;

        HolographicScene(Engine& core, winrt::Windows::Graphics::Holographic::HolographicSpace holographicSpace);

        winrt::Windows::Graphics::Holographic::HolographicFrame CurrentFrame() const;
        winrt::Windows::Graphics::Holographic::HolographicSpace HolographicSpace() const;

        winrt::Windows::Perception::Spatial::SpatialCoordinateSystem WorldCoordinateSystem() const;
        winrt::Windows::Perception::PerceptionTimestamp CurrentTimestamp() const;

        void UpdateCurrentPrediction();

        void AddPredictionUpdateListener(std::shared_ptr<IPredictionUpdateListener> listener);
        void RemovePredictionUpdateListener(std::shared_ptr<IPredictionUpdateListener> listener);

    protected:
        void Initialize() override;
        void Update(float) override;
        void Uninitialize() override;

        void OnCurrentStageChanged();

        void OnPredictionChanged(IPredictionUpdateListener::PredictionUpdateReason reason);

    private:
        mutable std::shared_mutex m_mutex;
        winrt::Windows::Perception::Spatial::SpatialStageFrameOfReference m_stageFrameOfReference{ nullptr };
        winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference m_stationaryFrameOfReference{ nullptr };
        winrt::event_token m_spatialStageCurrentChanged;

        winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace{ nullptr };
        winrt::Windows::Graphics::Holographic::HolographicFrame m_currentFrame{ nullptr };

        ListenerCollection<IPredictionUpdateListener> m_predictionUpdatelisteners;
    };

}
////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Engine\ListenerCollection.h>

namespace Neso 
{
    ////////////////////////////////////////////////////////////////////////////////
    // SpatialInteraction event listener
    class ISpatialInteractionListener abstract
    {
    public:
        virtual void OnSourceDetected(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) {};

        virtual void OnSourceLost(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) {};

        virtual void OnSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) {};

        virtual void OnSourceUpdated(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) {};

        virtual void OnSourceReleased(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args) {};
    };

    ////////////////////////////////////////////////////////////////////////////////
    // SpatialInteractionSystem
    // Responsible for managing the events from SpatialInteractionManager with additional filtering
    class SpatialInteractionSystem final : public System<SpatialInteractionSystem>
    {
    public:
        using System::System;

        void AddListener(std::shared_ptr<ISpatialInteractionListener> listener)
        {
            m_spatialInteractionListeners.Add(std::move(listener));
        }

        void RemoveListener(std::shared_ptr<ISpatialInteractionListener> listener)
        {
            m_spatialInteractionListeners.Remove(std::move(listener));
        }

        winrt::Windows::UI::Input::Spatial::ISpatialInteractionManager GetInteractionManager() const
        {
            fail_fast_if(m_spatialInteractionManager == nullptr);
            return m_spatialInteractionManager;
        }

    protected:
        void Initialize() override;
        void Uninitialize() override;

    private:
        winrt::Windows::UI::Input::Spatial::ISpatialInteractionManager m_spatialInteractionManager{ nullptr };

        enum SourceEvent {
            Detected, Pressed, Updated, Released, Lost, Count
        };

        winrt::event_token m_sourceTokens[SourceEvent::Count];

        ListenerCollection<ISpatialInteractionListener> m_spatialInteractionListeners;

        void BindEventHandlers();
        void ReleaseEventHandlers();

        // Events Handlers
        void HandleSourceDetected(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionManager& sender,
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs&  args);

        void HandleSourceLost(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionManager& sender,
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args);

        void HandleSourcePressed(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionManager& sender,
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args);

        void HandleSourceUpdated(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionManager& sender,
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args);

        void HandleSourceReleased(
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionManager& sender,
            const winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs& args);

    };

} // namespace Neso

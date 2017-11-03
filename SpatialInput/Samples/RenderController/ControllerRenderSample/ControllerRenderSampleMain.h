#pragma once

//
// Comment out this preprocessor definition to disable all of the
// sample content.
//
// To remove the content after disabling it:
//     * Remove the unused code from your app's Main class.
//     * Delete the Content folder provided with this template.
//
#define DRAW_SAMPLE_CONTENT

#include "Common\DeviceResources.h"
#include "Common\StepTimer.h"

#ifdef DRAW_SAMPLE_CONTENT
#include "Content\SpinningCubeRenderer.h"
#endif

#include <Pbr\PbrResources.h>

struct ControllerModelCache;

// Updates, renders, and presents holographic content using Direct3D.
namespace ControllerRenderSample
{
    class ControllerRenderSampleMain : public DX::IDeviceNotify
    {
    public:
        ControllerRenderSampleMain(std::shared_ptr<DX::DeviceResources> const& deviceResources);
        ~ControllerRenderSampleMain();

        // Sets the holographic space. This is our closest analogue to setting a new window
        // for the app.
        void SetHolographicSpace(winrt::Windows::Graphics::Holographic::HolographicSpace const& holographicSpace);

        // Starts the holographic frame and updates the content.
        winrt::Windows::Graphics::Holographic::HolographicFrame Update();

        // Renders holograms, including world-locked content.
        bool Render(winrt::Windows::Graphics::Holographic::HolographicFrame const& holographicFrame);

        // Handle saving and loading of app state owned by AppMain.
        void SaveAppState();
        void LoadAppState();

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

    private:
        void RenderControllers(winrt::Windows::Perception::PerceptionTimestamp const& timestamp);

        std::future<void> InitializeAsync();

        void OnSourcePressed(
            winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
            winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

        // Asynchronously creates resources for new holographic cameras.
        void OnCameraAdded(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
            winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const& args);

        // Synchronously releases resources for holographic cameras that are no longer
        // attached to the system.
        void OnCameraRemoved(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
            winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const& args);

        // Used to notify the app when the positional tracking state changes.
        void OnLocatabilityChanged(
            winrt::Windows::Perception::Spatial::SpatialLocator const& sender,
            winrt::Windows::Foundation::IInspectable const& args);

        // Clears event registration state. Used when changing to a new HolographicSpace
        // and when tearing down AppMain.
        void UnregisterHolographicEventHandlers();

        // API objects used to process gesture input, and generate gesture events.
        winrt::Windows::UI::Input::Spatial::SpatialInteractionManager m_interactionManager{ nullptr };
        winrt::event_token                                          m_sourcePressedEventToken;

        std::shared_ptr<ControllerModelCache>                       m_controllerModelCache;

#ifdef DRAW_SAMPLE_CONTENT
        // Renders a colorful holographic cube that's 20 centimeters wide. This sample content
        // is used to demonstrate world-locked rendering.
        std::unique_ptr<SpinningCubeRenderer>                       m_spinningCubeRenderer;
#endif

        // DX resources required for PBR rendering.
        std::shared_ptr<Pbr::Resources>                             m_pbrResources;

        // Set to true when all resources have been loaded and rendering/update logic can run.
        bool                                                        m_loadingComplete{ false };

        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources>                        m_deviceResources;

        // Render loop timer.
        DX::StepTimer                                               m_timer;

        // Represents the holographic space around the user.
        winrt::Windows::Graphics::Holographic::HolographicSpace     m_holographicSpace = nullptr;

        // SpatialLocator that is attached to the primary camera.
        winrt::Windows::Perception::Spatial::SpatialLocator         m_locator = nullptr;

        // A reference frame attached to the holographic camera.
        winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference m_referenceFrame = nullptr;

        // Event registration tokens.
        winrt::event_token                                          m_cameraAddedToken;
        winrt::event_token                                          m_cameraRemovedToken;
        winrt::event_token                                          m_locatabilityChangedToken;
    };
}

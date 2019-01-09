////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include <DirectXTK\DDSTextureLoader.h>
#include "Common\DirectXHelper.h"
#include "ControllerRenderSampleMain.h"

using namespace ControllerRenderSample;

using namespace concurrency;
using namespace DirectX;
using namespace std::placeholders;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

// Loads and initializes application assets when the application is loaded.
ControllerRenderSampleMain::ControllerRenderSampleMain(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
    // Register to be notified if the device is lost or recreated.
    m_deviceResources->RegisterDeviceNotify(this);
}

void ControllerRenderSampleMain::SetHolographicSpace(HolographicSpace const& holographicSpace)
{
    UnregisterHolographicEventHandlers();

    m_holographicSpace = holographicSpace;

    m_pbrResources = std::make_shared<Pbr::Resources>(m_deviceResources->GetD3DDevice());

    // Set up a light source (an image-based lighting environment map will also be loaded and contribute to the scene lighting).
    m_pbrResources->SetLight(XMVECTORF32{ 0.0f, 0.7071067811865475f, 0.7071067811865475f }, XMVECTORF32{ 1.0f, 0.8f, 0.7f } /* Roughly an incandescent light color */ );

    // Start initialization but don't wait for it to finish.
    (void) InitializePbrResourcesAsync();

    // The interaction manager provides an event that informs the app when
    // spatial interactions are detected.
    m_interactionManager = SpatialInteractionManager::GetForCurrentView();

    m_controllerRenderer = std::make_unique<ControllerRenderer>(m_deviceResources, m_pbrResources, m_interactionManager);

    // Initialize the sample hologram.
    m_spinningCubeRenderer = std::make_unique<SpinningCubeRenderer>(m_deviceResources, m_pbrResources);

    // Bind a handler to the SourcePressed event.
    m_sourcePressedEventToken = m_interactionManager.SourcePressed(bind(&ControllerRenderSampleMain::OnSourcePressed, this, _1, _2));

    // Use the default SpatialLocator to track the motion of the device.
    m_locator = SpatialLocator::GetDefault();

    // Be able to respond to changes in the positional tracking state.
    m_locatabilityChangedToken = m_locator.LocatabilityChanged(std::bind(&ControllerRenderSampleMain::OnLocatabilityChanged, this, _1, _2));

    // Respond to camera added events by creating any resources that are specific
    // to that camera, such as the back buffer render target view.
    // When we add an event handler for CameraAdded, the API layer will avoid putting
    // the new camera in new HolographicFrames until we complete the deferral we created
    // for that handler, or return from the handler without creating a deferral. This
    // allows the app to take more than one frame to finish creating resources and
    // loading assets for the new holographic camera.
    // This function should be registered before the app creates any HolographicFrames.
    m_cameraAddedToken = m_holographicSpace.CameraAdded(std::bind(&ControllerRenderSampleMain::OnCameraAdded, this, _1, _2));

    // Respond to camera removed events by releasing resources that were created for that
    // camera.
    // When the app receives a CameraRemoved event, it releases all references to the back
    // buffer right away. This includes render target views, Direct2D target bitmaps, and so on.
    // The app must also ensure that the back buffer is not attached as a render target, as
    // shown in DeviceResources::ReleaseResourcesForBackBuffer.
    m_cameraRemovedToken = m_holographicSpace.CameraRemoved(std::bind(&ControllerRenderSampleMain::OnCameraRemoved, this, _1, _2));

    // The simplest way to render world-locked holograms is to create a stationary reference frame
    // when the app is launched. This is roughly analogous to creating a "world" coordinate system
    // with the origin placed at the device's position as the app is launched.
    m_referenceFrame = m_locator.CreateStationaryFrameOfReferenceAtCurrentLocation();
}

void ControllerRenderSampleMain::UnregisterHolographicEventHandlers()
{
    if (m_holographicSpace != nullptr)
    {
        // Clear previous event registrations.
        m_holographicSpace.CameraAdded(m_cameraAddedToken);
        m_cameraAddedToken = {};

        m_holographicSpace.CameraRemoved(m_cameraRemovedToken);
        m_cameraRemovedToken = {};
    }

    if (m_locator != nullptr)
    {
        m_locator.LocatabilityChanged(m_locatabilityChangedToken);
        m_locatabilityChangedToken = {};
    }
}

ControllerRenderSampleMain::~ControllerRenderSampleMain()
{
    // Deregister device notification.
    m_deviceResources->RegisterDeviceNotify(nullptr);

    UnregisterHolographicEventHandlers();

    if (m_interactionManager)
    {
        m_interactionManager.SourcePressed(m_sourcePressedEventToken);
    }
}

// Updates the application state once per frame.
HolographicFrame ControllerRenderSampleMain::Update()
{
    if (!m_loadingComplete)
    {
        return nullptr;
    }

    // Before doing the timer update, there is some work to do per-frame
    // to maintain holographic rendering. First, we will get information
    // about the current frame.

    // The HolographicFrame has information that the app needs in order
    // to update and render the current frame. The app begins each new
    // frame by calling CreateNextFrame.
    HolographicFrame holographicFrame = m_holographicSpace.CreateNextFrame();

    // Get a prediction of where holographic cameras will be when this frame
    // is presented.
    HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();

    // Back buffers can change from frame to frame. Validate each buffer, and recreate
    // resource views and depth buffers as needed.
    m_deviceResources->EnsureCameraResources(holographicFrame, prediction);

    // Next, we get a coordinate system from the attached frame of reference that is
    // associated with the current frame. Later, this coordinate system is used for
    // creating the stereo view matrices when rendering the sample content.
    SpatialCoordinateSystem currentCoordinateSystem = m_referenceFrame.CoordinateSystem();

    m_timer.Tick([&] ()
    {
        // Run time-based updates.
        m_spinningCubeRenderer->Update(m_timer);
    });

    // We complete the frame update by using information about our content positioning
    // to set the focus point.

    for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
    {
        // The HolographicCameraRenderingParameters class provides access to set
        // the image stabilization parameters.
        HolographicCameraRenderingParameters renderingParameters = holographicFrame.GetRenderingParameters(cameraPose);

        // SetFocusPoint informs the system about a specific point in your scene to
        // prioritize for image stabilization. The focus point is set independently
        // for each holographic camera.
        // You should set the focus point near the content that the user is looking at.
        // In this example, we put the focus point at the center of the sample hologram,
        // since that is the only hologram available for the user to focus on.
        // You can also set the relative velocity and facing of that content; the sample
        // hologram is at a fixed point so we only need to indicate its position.
        renderingParameters.SetFocusPoint(
            currentCoordinateSystem,
            m_spinningCubeRenderer->GetPosition()
            );
    }

    // The holographic frame will be used to get up-to-date view and projection matrices and
    // to present the swap chain.
    return holographicFrame;
}

// Renders the current frame to each holographic camera, according to the
// current application and spatial positioning state. Returns true if the
// frame was rendered to at least one camera.
bool ControllerRenderSampleMain::Render(HolographicFrame const& holographicFrame)
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

    // Lock the set of holographic camera resources, then draw to each camera
    // in this frame.
    return m_deviceResources->UseHolographicCameraResources<bool>(
        [this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
    {
        // Up-to-date frame predictions enhance the effectiveness of image stablization and
        // allow more accurate positioning of holograms.
        holographicFrame.UpdateCurrentPrediction();
        HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();

        bool atLeastOneCameraRendered = false;
        for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();

            // Get the device context.
            const auto context = m_deviceResources->GetD3DDeviceContext();
            const auto depthStencilView = pCameraResources->GetDepthStencilView();

            // Set render targets to the current holographic camera.
            ID3D11RenderTargetView *const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
            context->OMSetRenderTargets(1, targets, depthStencilView);

            // Clear the back buffer and depth stencil view.
            context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
            context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            const SpatialCoordinateSystem coordinateSystem = m_referenceFrame.CoordinateSystem();

            // The view and projection matrices for each holographic camera will change
            // every frame. This function refreshes the data in the constant buffer for
            // the holographic camera indicated by cameraPose.
            // every frame. This function will return false when positional tracking is lost.
            HolographicStereoTransform viewTransform;
            HolographicStereoTransform projectionTransform;
            bool cameraActive = pCameraResources->GetViewProjectionTransform(m_deviceResources, cameraPose, coordinateSystem, &viewTransform, &projectionTransform);

            // Only render world-locked content when positional tracking is active.
            if (cameraActive)
            {
                // Attach the view/projection constant buffer for this camera to the graphics pipeline.
                m_pbrResources->SetViewProjection(
                    XMLoadFloat4x4(&viewTransform.Left),
                    XMLoadFloat4x4(&viewTransform.Right),
                    XMLoadFloat4x4(&projectionTransform.Left),
                    XMLoadFloat4x4(&projectionTransform.Right));

                m_pbrResources->Bind(m_deviceResources->GetD3DDeviceContext());

                m_controllerRenderer->Render(coordinateSystem, prediction.Timestamp());

                // Only render world-locked content when positional tracking is active.
                if (cameraActive)
                {
                    // Draw the sample hologram.
                    m_spinningCubeRenderer->Render();
                }

                // Provide depth buffer to the system for image stablization.
                pCameraResources->CommitDirect3D11DepthBuffer(holographicFrame.GetRenderingParameters(cameraPose));
            }

            atLeastOneCameraRendered = true;
        }

        return atLeastOneCameraRendered;
    });
}

void ControllerRenderSampleMain::SaveAppState()
{
}

void ControllerRenderSampleMain::LoadAppState()
{
}

// Interaction event handler.
void ControllerRenderSampleMain::OnSourcePressed(SpatialInteractionManager const& sender, SpatialInteractionSourceEventArgs const& args)
{
    if (const SpatialInteractionSourceLocation sourceLocation = args.State().Properties().TryGetLocation(m_referenceFrame.CoordinateSystem()))
    {
        if (const SpatialPointerInteractionSourcePose sourcePointerPose = sourceLocation.SourcePointerPose())
        {
            // Move the spinning cube in front of the controller when a button is pressed.
            constexpr float ForwardDistance = 0.3f;
            const float3 frontOfController = sourcePointerPose.Position() + (sourcePointerPose.ForwardDirection() * ForwardDistance);
            m_spinningCubeRenderer->SetPosition(frontOfController);
            m_spinningCubeRenderer->SetOrientation(sourcePointerPose.Orientation());
        }
    }
}

// Notifies classes that use Direct3D device resources that the device resources
// need to be released before this method returns.
void ControllerRenderSampleMain::OnDeviceLost()
{
    if (m_spinningCubeRenderer)
    {
        m_spinningCubeRenderer->ReleaseDeviceDependentResources();
    }

    if (m_controllerRenderer)
    {
        m_controllerRenderer->ReleaseDeviceDependentResources();
    }

    if (m_pbrResources)
    {
        m_pbrResources->ReleaseDeviceDependentResources();
        m_loadingComplete = false;
    }
}

// Notifies classes that use Direct3D device resources that the device resources
// may now be recreated.
void ControllerRenderSampleMain::OnDeviceRestored()
{
    if (m_pbrResources)
    {
        m_pbrResources->CreateDeviceDependentResources(m_deviceResources->GetD3DDevice());

        // The BRDF LUT and environment map need to be reloaded.
        (void) InitializePbrResourcesAsync();
    }

    if (m_controllerRenderer)
    {
        m_controllerRenderer->CreateDeviceDependentResources();
    }

    if (m_spinningCubeRenderer)
    {
        m_spinningCubeRenderer->CreateDeviceDependentResources();
    }
}

winrt::Windows::Foundation::IAsyncAction ControllerRenderSampleMain::InitializePbrResourcesAsync()
{
    // Read the BRDF Lookup Table used by the PBR system into a DirectX texture.
    std::vector<byte> brdfLutFileData = co_await DX::ReadDataAsync(L"ms-appx:///PBR/brdf_lut.png");
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brdLutResourceView = Pbr::Texture::LoadImage(
        m_deviceResources->GetD3DDevice(),
        brdfLutFileData.data(),
        (uint32_t)brdfLutFileData.size());
    m_pbrResources->SetBrdfLut(brdLutResourceView.Get());

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTextureView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTextureView;
#ifdef NO_ENVIRONMENT_IBL
    diffuseTextureView = Pbr::Texture::CreateFlatCubeTexture(m_deviceResources->GetD3DDevice(), DirectX::Colors::White);
    specularTextureView = Pbr::Texture::CreateFlatCubeTexture(m_deviceResources->GetD3DDevice(), DirectX::Colors::White);
#else
    winrt::check_hresult(DirectX::CreateDDSTextureFromFile(
        m_deviceResources->GetD3DDevice(),
        L"Media\\Environment\\DiffuseHDR.DDS",
        nullptr,
        &diffuseTextureView));

    winrt::check_hresult(DirectX::CreateDDSTextureFromFile(
        m_deviceResources->GetD3DDevice(),
        L"Media\\Environment\\SpecularHDR.DDS",
        nullptr,
        &specularTextureView));
#endif

    m_pbrResources->SetEnvironmentMap(
        m_deviceResources->GetD3DDeviceContext(),
        specularTextureView.Get(),
        diffuseTextureView.Get());

    m_loadingComplete = true;
}

void ControllerRenderSampleMain::OnLocatabilityChanged(SpatialLocator const& sender, winrt::Windows::Foundation::IInspectable const& args)
{
    switch (sender.Locatability())
    {
    case SpatialLocatability::Unavailable:
        // Holograms cannot be rendered.
        {
            winrt::hstring message = L"Warning! Positional tracking is " + std::to_wstring(int(sender.Locatability())) + L".\n";
            OutputDebugStringW(message.data());
        }
        break;

    // In the following three cases, it is still possible to place holograms using a
    // SpatialLocatorAttachedFrameOfReference.
    case SpatialLocatability::PositionalTrackingActivating:
        // The system is preparing to use positional tracking.

    case SpatialLocatability::OrientationOnly:
        // Positional tracking has not been activated.

    case SpatialLocatability::PositionalTrackingInhibited:
        // Positional tracking is temporarily inhibited. User action may be required
        // in order to restore positional tracking.
        break;

    case SpatialLocatability::PositionalTrackingActive:
        // Positional tracking is active. World-locked content can be rendered.
        break;
    }
}

void ControllerRenderSampleMain::OnCameraAdded(
    HolographicSpace const& sender,
    HolographicSpaceCameraAddedEventArgs const& args
    )
{
    winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
    HolographicCamera holographicCamera = args.Camera();
    create_task([this, deferral, holographicCamera] ()
    {
        // Create device-based resources for the holographic camera and add it to the list of
        // cameras used for updates and rendering. Notes:
        //   * Since this function may be called at any time, the AddHolographicCamera function
        //     waits until it can get a lock on the set of holographic camera resources before
        //     adding the new camera. At 60 frames per second this wait should not take long.
        //   * A subsequent Update will take the back buffer from the RenderingParameters of this
        //     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
        //     Content can then be rendered for the HolographicCamera.
        m_deviceResources->AddHolographicCamera(holographicCamera);

        // Holographic frame predictions will not include any information about this camera until
        // the deferral is completed.
        deferral.Complete();
    });
}

void ControllerRenderSampleMain::OnCameraRemoved(
    HolographicSpace const& sender,
    HolographicSpaceCameraRemovedEventArgs const& args
    )
{
    // Before letting this callback return, ensure that all references to the back buffer 
    // are released.
    // Since this function may be called at any time, the RemoveHolographicCamera function
    // waits until it can get a lock on the set of holographic camera resources before
    // deallocating resources for this camera. At 60 frames per second this wait should
    // not take long.
    m_deviceResources->RemoveHolographicCamera(args.Camera());
}


////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "HolographicRenderer.h"
#include "CommonComponents.h"
#include "EntityStore.h"
#include "HolographicScene.h"

#include "TextRenderer.h"
#include "QuadRenderer.h"
#include "SkyboxRenderer.h"

#include "Common\DirectXHelper.h"

#include "Pbr\PbrModel.h"
#include "Pbr\PbrMaterial.h"
#include "Pbr\PbrPrimitive.h"

using namespace Neso;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace DirectX;

#include "DirectXTK\PlatformHelpers.h"

////////////////////////////////////////////////////////////////////////////////
// Holographic Renderer 
////////////////////////////////////////////////////////////////////////////////
HolographicRenderer::HolographicRenderer(
    Engine& core,
    std::shared_ptr<DX::DeviceResources> deviceResources,
    std::shared_ptr<Pbr::Resources> pbrResources,
    ID3D11ShaderResourceView* skyboxTexture) :
    System(core),
    m_deviceResources(std::move(deviceResources)),
    m_pbrResources(std::move(pbrResources))
{
    m_quadRenderer = std::make_unique<QuadRenderer>(m_deviceResources);
    m_skyboxRenderer = std::make_unique<SkyboxRenderer>(m_deviceResources, skyboxTexture);
}

HolographicRenderer::~HolographicRenderer() = default;

std::shared_ptr<Pbr::Resources> HolographicRenderer::GetPbrResources()
{
    fail_fast_if(m_pbrResources == nullptr);
    return m_pbrResources;
}

std::shared_ptr<DX::DeviceResources> HolographicRenderer::GetDeviceResources()
{
    fail_fast_if(m_deviceResources == nullptr);
    return m_deviceResources;
}

void HolographicRenderer::OnDeviceLost()
{
    m_pbrResources->ReleaseDeviceDependentResources();
    for (auto& rendererPair : m_textRenderers)
    {
        rendererPair.second->ReleaseDeviceDependentResources();
    }
    m_quadRenderer->ReleaseDeviceDependentResources();
    m_skyboxRenderer->ReleaseDeviceDependentResources();
}

void HolographicRenderer::OnDeviceRestored()
{
    m_pbrResources->CreateDeviceDependentResources(m_deviceResources->GetD3DDevice());
    for (auto& rendererPair : m_textRenderers)
    {
        rendererPair.second->CreateDeviceDependentResources();
    }
    m_quadRenderer->CreateDeviceDependentResources();
    m_skyboxRenderer->CreateDeviceDependentResources();
}

void HolographicRenderer::Initialize()
{
    m_deviceResources->RegisterDeviceNotify(this);
    m_pbrResources->SetLight(XMVECTORF32{ 0.0f, 0.7071067811865475f, -0.7071067811865475f }, Colors::White);
}

void HolographicRenderer::Uninitialize()
{
    m_deviceResources->RegisterDeviceNotify(nullptr);
}

void HolographicRenderer::Start()
{
    m_entityStore = m_engine.Get<EntityStore>();
    m_holoScene = m_engine.Get<HolographicScene>();

    BindEventHandlers(m_holoScene->HolographicSpace());
}

void HolographicRenderer::Stop()
{
    ReleaseEventHandlers(m_holoScene->HolographicSpace());

    m_holoScene = nullptr;
    m_entityStore = nullptr;
}

void HolographicRenderer::Update(float)
{
    auto holographicFrame = m_holoScene->CurrentFrame();

    m_deviceResources->EnsureCameraResources(holographicFrame, holographicFrame.CurrentPrediction());

    const bool shouldPresent = m_deviceResources->UseHolographicCameraResources<bool>(
        [this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
    {
        // Up-to-date frame predictions enhance the effectiveness of image stablization and
        // allow more accurate positioning of holograms.
        m_holoScene->UpdateCurrentPrediction();

        HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();
        SpatialCoordinateSystem coordinateSystem = m_holoScene->WorldCoordinateSystem();

        bool atLeastOneCameraRendered = false;
        for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();

            if (RenderAtCameraPose(pCameraResources, coordinateSystem, prediction, holographicFrame.GetRenderingParameters(cameraPose), cameraPose))
            {
                atLeastOneCameraRendered = true;
            }
        }

        return atLeastOneCameraRendered;
    });

    if (shouldPresent)
    {
        m_deviceResources->Present(holographicFrame);
    }
}

TextRenderer* HolographicRenderer::GetTextRendererForFontSize(float fontSize)
{
    auto it = m_textRenderers.find(fontSize);
    if (it == m_textRenderers.end())
    {
        auto textRenderer = std::make_unique<TextRenderer>(m_deviceResources, 1024u, 1024u, fontSize);
        it = m_textRenderers.insert(it, { fontSize, std::move(textRenderer) });
    }

    return it->second.get();
}

bool HolographicRenderer::RenderAtCameraPose(
    DX::CameraResources *pCameraResources,
    winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem,
    winrt::Windows::Graphics::Holographic::HolographicFramePrediction& prediction,
    winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const& renderingParameters,
    winrt::Windows::Graphics::Holographic::HolographicCameraPose const& cameraPose)
{
    // Get the device context.
    const auto context = m_deviceResources->GetD3DDeviceContext();
    const auto depthStencilView = pCameraResources->GetDepthStencilView();

    // Set render targets to the current holographic camera.
    ID3D11RenderTargetView *const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, depthStencilView);

    // TODO don't need to do this if we have a skybox
    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // The view and projection matrices for each holographic camera will change
    // every frame. This function will return false when positional tracking is lost.
    HolographicStereoTransform coordinateSystemToView;
    HolographicStereoTransform viewToProjection;
    bool cameraActive = pCameraResources->GetViewProjectionTransform(m_deviceResources, cameraPose, coordinateSystem, &coordinateSystemToView, &viewToProjection);

    // Only render world-locked content when positional tracking is active.
    if (cameraActive)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Pbr Rendering
        m_pbrResources->SetViewProjection(
            XMLoadFloat4x4(&coordinateSystemToView.Left),
            XMLoadFloat4x4(&coordinateSystemToView.Right),
            XMLoadFloat4x4(&viewToProjection.Left),
            XMLoadFloat4x4(&viewToProjection.Right));

        m_pbrResources->Bind(m_deviceResources->GetD3DDeviceContext());

        for (auto[transform, pbr] : m_entityStore->GetComponents<Transform, PbrRenderable>())
        {
            if (pbr->Model)
            {
                float4x4 transformMtx = transform->GetMatrix();

                if (pbr->Offset)
                {
                    transformMtx = *pbr->Offset * transformMtx;
                }

                pbr->Model->GetNode(Pbr::RootNodeIndex).SetTransform(XMLoadFloat4x4(&transformMtx));
                pbr->Model->Render(*m_pbrResources, m_deviceResources->GetD3DDeviceContext());
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Text Rendering
        m_quadRenderer->SetViewProjection(
            coordinateSystemToView.Left,
            viewToProjection.Left,
            coordinateSystemToView.Right,
            viewToProjection.Right);

        m_quadRenderer->Bind();

        float prevFontSize = std::numeric_limits<float>::quiet_NaN();
        TextRenderer* textRenderer = nullptr;

        for (auto[transform, textRenderable] : m_entityStore->GetComponents<Transform, TextRenderable>())
        {
            if (prevFontSize != textRenderable->FontSize)
            {
                prevFontSize = textRenderable->FontSize;
                textRenderer = GetTextRendererForFontSize(prevFontSize);
            }

            textRenderer->RenderTextOffscreen(textRenderable->Text);
            m_quadRenderer->Render(transform->GetMatrix(), textRenderer->GetTexture());
        }

        m_quadRenderer->Unbind();

        ////////////////////////////////////////////////////////////////////////////////
        // Skybox Rendering
        float4x4 cameraToCoordinateSystem = float4x4::identity();
        if (auto location = SpatialLocator::GetDefault().TryLocateAtTimestamp(prediction.Timestamp(), coordinateSystem))
        {
            cameraToCoordinateSystem = make_float4x4_translation(location.Position());
        }

        m_skyboxRenderer->SetViewProjection(
            cameraToCoordinateSystem * coordinateSystemToView.Left,  viewToProjection.Left,
            cameraToCoordinateSystem * coordinateSystemToView.Right, viewToProjection.Right);

        m_skyboxRenderer->Bind();
        m_skyboxRenderer->Render();
        m_skyboxRenderer->Unbind();

        pCameraResources->CommitDirect3D11DepthBuffer(renderingParameters);
    }

    return true;
}

void HolographicRenderer::BindEventHandlers(
    const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace)
{
    fail_fast_if(holographicSpace == nullptr);

    m_cameraAddedToken = holographicSpace.CameraAdded(
        std::bind(&HolographicRenderer::OnCameraAdded, this, std::placeholders::_1, std::placeholders::_2));

    m_cameraRemovedToken = holographicSpace.CameraRemoved(
        std::bind(&HolographicRenderer::OnCameraRemoved, this, std::placeholders::_1, std::placeholders::_2));
}

void HolographicRenderer::ReleaseEventHandlers(
    const winrt::Windows::Graphics::Holographic::HolographicSpace& holographicSpace)
{
    fail_fast_if(holographicSpace == nullptr);

    holographicSpace.CameraRemoved(m_cameraRemovedToken);
    holographicSpace.CameraAdded(m_cameraAddedToken);
}

// Asynchronously creates resources for new holographic cameras.
void HolographicRenderer::OnCameraAdded(
    winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const& args)
{
    winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
    HolographicCamera holographicCamera = args.Camera();
    concurrency::create_task([this, deferral, holographicCamera]()
    {
        //
        // TODO: Allocate resources for the new camera and load any content specific to
        //       that camera. Note that the render target size (in pixels) is a property
        //       of the HolographicCamera object, and can be used to create off-screen
        //       render targets that match the resolution of the HolographicCamera.
        //

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

// Synchronously releases resources for holographic cameras that are no longer
// attached to the system.

void HolographicRenderer::OnCameraRemoved(
    winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
    winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const& args)
{
    concurrency::create_task([this]()
    {
        //
        // TODO: Asynchronously unload or deactivate content resources (not back buffer
        //       resources) that are specific only to the camera that was removed.
        //
    });

    // Before letting this callback return, ensure that all references to the back buffer
    // are released.
    // Since this function may be called at any time, the RemoveHolographicCamera function
    // waits until it can get a lock on the set of holographic camera resources before
    // deallocating resources for this camera. At 60 frames per second this wait should
    // not take long.
    m_deviceResources->RemoveHolographicCamera(args.Camera());
}

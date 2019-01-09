////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"

#include "CameraResources.h"
#include "Common\DirectXHelper.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception::Spatial;

DX::CameraResources::CameraResources(HolographicCamera const& camera) :
    m_holographicCamera(camera),
    m_isStereo(camera.IsStereo()),
    m_d3dRenderTargetSize(camera.RenderTargetSize())
{
    m_d3dViewport = CD3D11_VIEWPORT(
        0.f, 0.f,
        m_d3dRenderTargetSize.Width,
        m_d3dRenderTargetSize.Height
        );
};

// Updates resources associated with a holographic camera's swap chain.
// The app does not access the swap chain directly, but it does create
// resource views for the back buffer.
void DX::CameraResources::CreateResourcesForBackBuffer(
    const DX::DeviceResources* pDeviceResources,
    HolographicCameraRenderingParameters const& cameraParameters
    )
{
    ID3D11Device* device = pDeviceResources->GetD3DDevice();

    // Get the WinRT object representing the holographic camera's back buffer.
    IDirect3DSurface surface = cameraParameters.Direct3D11BackBuffer();

    // Get the holographic camera's back buffer.
    // Holographic apps do not create a swap chain themselves; instead, buffers are
    // owned by the system. The Direct3D back buffer resources are provided to the
    // app using WinRT interop APIs.
    ComPtr<ID3D11Texture2D> cameraBackBuffer;
    winrt::check_hresult(surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>()->GetInterface(IID_PPV_ARGS(&cameraBackBuffer)));

    // Determine if the back buffer has changed. If so, ensure that the render target view
    // is for the current back buffer.
    if (m_d3dBackBuffer.Get() != cameraBackBuffer.Get())
    {
        // This can change every frame as the system moves to the next buffer in the
        // swap chain. This mode of operation will occur when certain rendering modes
        // are activated.
        m_d3dBackBuffer = cameraBackBuffer;

        // Create a render target view of the back buffer.
        // Creating this resource is inexpensive, and is better than keeping track of
        // the back buffers in order to pre-allocate render target views for each one.
        winrt::check_hresult(
            device->CreateRenderTargetView(
                m_d3dBackBuffer.Get(),
                nullptr,
                &m_d3dRenderTargetView
            ));

        // Get the DXGI format for the back buffer.
        // This information can be accessed by the app using CameraResources::GetBackBufferDXGIFormat().
        D3D11_TEXTURE2D_DESC backBufferDesc;
        m_d3dBackBuffer->GetDesc(&backBufferDesc);
        m_dxgiFormat = backBufferDesc.Format;

        // Check for render target size changes.
        winrt::Windows::Foundation::Size currentSize = m_holographicCamera.RenderTargetSize();
        if (m_d3dRenderTargetSize != currentSize)
        {
            // Set render target size.
            m_d3dRenderTargetSize = currentSize;

            // A new depth stencil view is also needed.
            m_d3dDepthStencilView.Reset();
        }
    }

    // Refresh depth stencil resources, if needed.
    if (m_d3dDepthStencilView == nullptr)
    {
        // Create a depth stencil view for use with 3D rendering if needed.
        CD3D11_TEXTURE2D_DESC depthStencilDesc(
            DXGI_FORMAT_R16_TYPELESS,
            static_cast<UINT>(m_d3dRenderTargetSize.Width),
            static_cast<UINT>(m_d3dRenderTargetSize.Height),
            m_isStereo ? 2 : 1, // Create two textures when rendering in stereo.
            1, // Use a single mipmap level.
            D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
        );

        winrt::check_hresult(
            device->CreateTexture2D(
                &depthStencilDesc,
                nullptr,
                &m_d3dDepthStencil
            ));

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
            m_isStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_D16_UNORM
        );
        winrt::check_hresult(
            device->CreateDepthStencilView(
                m_d3dDepthStencil.Get(),
                &depthStencilViewDesc,
                &m_d3dDepthStencilView
            ));
    }
}

// Releases resources associated with a back buffer.
void DX::CameraResources::ReleaseResourcesForBackBuffer(const DX::DeviceResources* pDeviceResources)
{
    ID3D11DeviceContext* context = pDeviceResources->GetD3DDeviceContext();

    // Release camera-specific resources.
    m_d3dBackBuffer.Reset();
    m_d3dDepthStencil.Reset();
    m_d3dRenderTargetView.Reset();
    m_d3dDepthStencilView.Reset();

    // Ensure system references to the back buffer are released by clearing the render
    // target from the graphics pipeline state, and then flushing the Direct3D context.
    ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    context->Flush();
}

// Gets the view/projection transforms for a holographic camera.
bool DX::CameraResources::GetViewProjectionTransform(
    std::shared_ptr<DX::DeviceResources> deviceResources,
    HolographicCameraPose const& cameraPose,
    SpatialCoordinateSystem const& coordinateSystem,
    _Out_ HolographicStereoTransform* viewTransform,
    _Out_ HolographicStereoTransform* projectionTransform)
{
    // The system changes the viewport on a per-frame basis for system optimizations.
    auto viewport = cameraPose.Viewport();
    m_d3dViewport = CD3D11_VIEWPORT(
        viewport.X,
        viewport.Y,
        viewport.Width,
        viewport.Height
    );

    // The projection transform for each frame is provided by the HolographicCameraPose.
    *projectionTransform = cameraPose.ProjectionTransform();

    // Get a container object with the view and projection matrices for the given
    // pose in the given coordinate system.
    auto viewTransformContainer = cameraPose.TryGetViewTransform(coordinateSystem);

    // If TryGetViewTransform returns a null pointer, that means the pose and coordinate
    // system cannot be understood relative to one another; content cannot be rendered
    // in this coordinate system for the duration of the current frame.
    // This usually means that positional tracking is not active for the current frame, in
    // which case it is possible to use a SpatialLocatorAttachedFrameOfReference to render
    // content that is not world-locked instead.
    bool viewTransformAcquired = viewTransformContainer != nullptr;
    if (viewTransformAcquired)
    {
        // Otherwise, the set of view transforms can be retrieved.
        *viewTransform = viewTransformContainer.Value();

        deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &m_d3dViewport);

        return true;
    }

    return false;
}

void DX::CameraResources::CommitDirect3D11DepthBuffer(HolographicCameraRenderingParameters const& renderingParameters) const
{
    // Direct3D interop APIs are used to provide the buffer to the WinRT API.
    ComPtr<IDXGIResource1> depthStencilResource;
    winrt::check_hresult(m_d3dDepthStencil.As(&depthStencilResource));

    ComPtr<IDXGISurface2> depthDxgiSurface;
    winrt::check_hresult(depthStencilResource->CreateSubresourceSurface(0, &depthDxgiSurface));

    winrt::com_ptr<::IInspectable> inspectableSurface;
    winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(depthDxgiSurface.Get(), winrt::put_abi(inspectableSurface)));

    // Calling CommitDirect3D11DepthBuffer causes the system to queue Direct3D commands to 
    // read the depth buffer. It will then use that information to stabilize the image as
    // the HolographicFrame is presented.
    renderingParameters.CommitDirect3D11DepthBuffer(inspectableSurface.as<IDirect3DSurface>());
}

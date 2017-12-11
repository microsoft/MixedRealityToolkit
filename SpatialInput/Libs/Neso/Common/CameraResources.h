////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DX
{
    class DeviceResources;

    // Manages DirectX device resources that are specific to a holographic camera, such as the
    // back buffer, ViewProjection constant buffer, and viewport.
    class CameraResources
    {
    public:
        CameraResources(winrt::Windows::Graphics::Holographic::HolographicCamera const& holographicCamera);

        void CreateResourcesForBackBuffer(
            const DX::DeviceResources* pDeviceResources,
            winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const& cameraParameters
            );
        void ReleaseResourcesForBackBuffer(
            const DX::DeviceResources* pDeviceResources
            );

        bool GetViewProjectionTransform(
            std::shared_ptr<DX::DeviceResources> deviceResources,
            winrt::Windows::Graphics::Holographic::HolographicCameraPose const& cameraPose,
            winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem,
            _Out_ winrt::Windows::Graphics::Holographic::HolographicStereoTransform* viewTransform,
            _Out_ winrt::Windows::Graphics::Holographic::HolographicStereoTransform* projectionTransform);

        void CommitDirect3D11DepthBuffer(
            winrt::Windows::Graphics::Holographic::HolographicCameraRenderingParameters const& renderingParameters) const;

        // Direct3D device resources.
        ID3D11RenderTargetView* GetBackBufferRenderTargetView()     const { return m_d3dRenderTargetView.Get();     }
        ID3D11DepthStencilView* GetDepthStencilView()               const { return m_d3dDepthStencilView.Get();     }
        ID3D11Texture2D*        GetBackBufferTexture2D()            const { return m_d3dBackBuffer.Get();           }
        D3D11_VIEWPORT          GetViewport()                       const { return m_d3dViewport;                   }
        DXGI_FORMAT             GetBackBufferDXGIFormat()           const { return m_dxgiFormat;                    }

        // Render target properties.
        winrt::Windows::Foundation::Size GetRenderTargetSize()      const& { return m_d3dRenderTargetSize;           }
        bool                    IsRenderingStereoscopic()           const  { return m_isStereo;                      }

        // The holographic camera these resources are for.
        winrt::Windows::Graphics::Holographic::HolographicCamera const& GetHolographicCamera() const { return m_holographicCamera; }

    private:
        // Direct3D rendering objects. Required for 3D.
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>              m_d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>              m_d3dDepthStencilView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>                     m_d3dDepthStencil;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>                     m_d3dBackBuffer;

        // Direct3D rendering properties.
        DXGI_FORMAT                                                 m_dxgiFormat;
        winrt::Windows::Foundation::Size                            m_d3dRenderTargetSize;
        D3D11_VIEWPORT                                              m_d3dViewport;

        // Indicates whether the camera supports stereoscopic rendering.
        bool                                                        m_isStereo = false;

        // Pointer to the holographic camera these resources are for.
        winrt::Windows::Graphics::Holographic::HolographicCamera    m_holographicCamera = nullptr;
    };
}


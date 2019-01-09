////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrMaterial.h"

#include "Shaders\Compiled\PbrPixelShader.h"
#include "Shaders\Compiled\PbrVertexShaderVprt.h"
#include "Shaders\Compiled\PbrVertexShaderNoVprt.h"
#include "Shaders\Compiled\PbrGeometryShaderNoVprt.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace
{
    struct SceneConstantBuffer
    {
        alignas(16) DirectX::XMFLOAT4X4 ViewProjection[2];
        alignas(16) DirectX::XMFLOAT4 EyePosition[2];
        alignas(16) DirectX::XMFLOAT3 LightDirection{};
        alignas(16) DirectX::XMFLOAT3 LightDiffuseColor{};
        alignas(16) int NumSpecularMipLevels{ 1 };
    };
}

namespace Pbr
{
    struct Resources::Impl
    {
        void Initialize(_In_ ID3D11Device* device)
        {
            // Set up pixel shader.
            Internal::ThrowIfFailed(device->CreatePixelShader(g_psPbrMain, sizeof(g_psPbrMain), nullptr, &Resources.PixelShader));

            // Check for device support for the optional feature that allows setting the render target array index from the vertex shader stage.
            D3D11_FEATURE_DATA_D3D11_OPTIONS3 options{};
            device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options, sizeof(options));
            if (options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer)
            {
                // Set up vertex shader with VPRT support.
                Internal::ThrowIfFailed(device->CreateInputLayout(Pbr::Vertex::s_vertexDesc, ARRAYSIZE(Pbr::Vertex::s_vertexDesc), g_vsPbrVprtMain, sizeof(g_vsPbrVprtMain), &Resources.InputLayout));
                Internal::ThrowIfFailed(device->CreateVertexShader(g_vsPbrVprtMain, sizeof(g_vsPbrVprtMain), nullptr, &Resources.VertexShader));
            }
            else
            {
                // Set up vertex shader with geometry shader due to no VPRT support.
                Internal::ThrowIfFailed(device->CreateInputLayout(Pbr::Vertex::s_vertexDesc, ARRAYSIZE(Pbr::Vertex::s_vertexDesc), g_vsPbrNoVprtMain, sizeof(g_vsPbrNoVprtMain), &Resources.InputLayout));
                Internal::ThrowIfFailed(device->CreateVertexShader(g_vsPbrNoVprtMain, sizeof(g_vsPbrNoVprtMain), nullptr, &Resources.VertexShader));
                Internal::ThrowIfFailed(device->CreateGeometryShader(g_gsPbrNoVprtMain, sizeof(g_gsPbrNoVprtMain), nullptr, &Resources.GeometryShader));
            }

            // Set up the constant buffers.
            static_assert((sizeof(SceneConstantBuffer) % 16) == 0, "Constant Buffer must be divisible by 16 bytes");
            const CD3D11_BUFFER_DESC pbrConstantBufferDesc(sizeof(SceneConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            Internal::ThrowIfFailed(device->CreateBuffer(&pbrConstantBufferDesc, nullptr, &Resources.ConstantBuffer));

            // Samplers for environment map and BRDF.
            Resources.EnvironmentMapSampler = Texture::CreateSampler(device);
            Resources.BrdfSampler = Texture::CreateSampler(device);
        }

        struct DeviceResources
        {
            Microsoft::WRL::ComPtr<ID3D11SamplerState> BrdfSampler;
            Microsoft::WRL::ComPtr<ID3D11SamplerState> EnvironmentMapSampler;
            Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
            Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
            Microsoft::WRL::ComPtr<ID3D11GeometryShader> GeometryShader;
            Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
            Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BrdfLut;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpecularEnvironmentMap;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DiffuseEnvironmentMap;
            mutable std::map<uint32_t, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> SolidColorTextureCache;
        };

        DeviceResources Resources;
        TrackChanges<SceneConstantBuffer> SceneBuffer;
        uint32_t SceneChangeCountBookmark{ 0 };
    };

    /* static */
    Resources::Resources(_In_ ID3D11Device* device)
        : m_impl(std::make_shared<Impl>())
    {
        m_impl->Initialize(device);
    }

    void Resources::SetBrdfLut(_In_ ID3D11ShaderResourceView* brdfLut)
    {
        m_impl->Resources.BrdfLut = brdfLut;
    }

    void Resources::CreateDeviceDependentResources(_In_ ID3D11Device* device)
    {
        m_impl->Initialize(device);
    }

    void Resources::ReleaseDeviceDependentResources()
    {
        m_impl->Resources = {};
    }

    Microsoft::WRL::ComPtr<ID3D11Device> Resources::GetDevice() const
    {
        ComPtr<ID3D11Device> device;
        m_impl->Resources.ConstantBuffer->GetDevice(&device);
        return device;
    }

    void XM_CALLCONV Resources::SetLight(DirectX::XMVECTOR direction, DirectX::XMVECTOR diffuseColor)
    {
        m_impl->SceneBuffer.Set([&](SceneConstantBuffer& sceneBuffer) {
            XMStoreFloat3(&sceneBuffer.LightDirection, direction);
            XMStoreFloat3(&sceneBuffer.LightDiffuseColor, diffuseColor);
        });
    }

    void XM_CALLCONV Resources::SetViewProjection(FXMMATRIX viewLeft, CXMMATRIX viewRight, CXMMATRIX projectionLeft, CXMMATRIX projectionRight)
    {
        m_impl->SceneBuffer.Set([&](SceneConstantBuffer& sceneBuffer) {
            XMStoreFloat4x4(&sceneBuffer.ViewProjection[0], XMMatrixTranspose(XMMatrixMultiply(viewLeft, projectionLeft)));
            XMStoreFloat4x4(&sceneBuffer.ViewProjection[1], XMMatrixTranspose(XMMatrixMultiply(viewRight, projectionRight)));
            XMStoreFloat4(&sceneBuffer.EyePosition[0], XMMatrixInverse(nullptr, viewLeft).r[3]);
            XMStoreFloat4(&sceneBuffer.EyePosition[1], XMMatrixInverse(nullptr, viewRight).r[3]);
        });
    }

    void Resources::SetEnvironmentMap(ID3D11DeviceContext3* context, ID3D11ShaderResourceView* specularEnvironmentMap, ID3D11ShaderResourceView* diffuseEnvironmentMap)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        diffuseEnvironmentMap->GetDesc(&desc);
        if (desc.ViewDimension != D3D_SRV_DIMENSION_TEXTURECUBE)
        {
            throw std::exception("Diffuse Resource View Type is not D3D_SRV_DIMENSION_TEXTURECUBE");
        }

        specularEnvironmentMap->GetDesc(&desc);
        if (desc.ViewDimension != D3D_SRV_DIMENSION_TEXTURECUBE)
        {
            throw std::exception("Specular Resource View Type is not D3D_SRV_DIMENSION_TEXTURECUBE");
        }
        
        m_impl->SceneBuffer.Set([&](SceneConstantBuffer& sceneBuffer) {
            sceneBuffer.NumSpecularMipLevels = desc.TextureCube.MipLevels;
        });
        m_impl->Resources.SpecularEnvironmentMap = specularEnvironmentMap;
        m_impl->Resources.DiffuseEnvironmentMap = diffuseEnvironmentMap;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Resources::CreateSolidColorTexture(CXMVECTOR color) const
    {
        const std::array<uint8_t, 4> rgba = Texture::CreateRGBA(color);

        // Check cache to see if this flat texture already exists.
        const uint32_t colorKey = *reinterpret_cast<const uint32_t*>(rgba.data());
        auto textureIt = m_impl->Resources.SolidColorTextureCache.find(colorKey);
        if (textureIt != m_impl->Resources.SolidColorTextureCache.end())
        {
            return textureIt->second;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture = Pbr::Texture::CreateTexture(GetDevice().Get(), rgba.data(), 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
        m_impl->Resources.SolidColorTextureCache.insert(std::make_pair(colorKey, texture));
        return texture;
    }

    void Resources::Bind(_In_ ID3D11DeviceContext3* context) const
    {
        // If the constant buffer parameters changed, update the D3D constant buffer.
        if (m_impl->SceneBuffer.UpdateChangeCountBookmark(&m_impl->SceneChangeCountBookmark))
        {
            context->UpdateSubresource(m_impl->Resources.ConstantBuffer.Get(), 0, nullptr, &m_impl->SceneBuffer, 0, 0);
        }

        context->VSSetShader(m_impl->Resources.VertexShader.Get(), nullptr, 0);
        context->PSSetShader(m_impl->Resources.PixelShader.Get(), nullptr, 0);
        context->GSSetShader(m_impl->Resources.GeometryShader.Get(), nullptr, 0);

        ID3D11Buffer* buffers[] = { m_impl->Resources.ConstantBuffer.Get() };
        context->VSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Scene, _countof(buffers), buffers);
        context->PSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Scene, _countof(buffers), buffers);
        context->IASetInputLayout(m_impl->Resources.InputLayout.Get());

        static_assert(ShaderSlots::DiffuseTexture == ShaderSlots::SpecularTexture + 1, "Diffuse must follow Specular slot");
        static_assert(ShaderSlots::SpecularTexture == ShaderSlots::Brdf + 1, "Specular must follow BRDF slot");
        ID3D11ShaderResourceView* shaderResources[] = { m_impl->Resources.BrdfLut.Get(), m_impl->Resources.SpecularEnvironmentMap.Get(), m_impl->Resources.DiffuseEnvironmentMap.Get() };
        context->PSSetShaderResources(Pbr::ShaderSlots::Brdf, _countof(shaderResources), shaderResources);
        ID3D11SamplerState* samplers[] = { m_impl->Resources.BrdfSampler.Get(), m_impl->Resources.EnvironmentMapSampler.Get() };
        context->PSSetSamplers(ShaderSlots::Brdf, _countof(samplers), samplers);
    }
}

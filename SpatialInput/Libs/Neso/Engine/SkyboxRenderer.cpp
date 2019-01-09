////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "SkyboxRenderer.h"
#include "EngineCommon.h"

#include <DirectXTK\PlatformHelpers.h>

#include "Common\DirectXHelper.h"

#include "Shaders\Compiled\SkyBoxPixelShader.h"
#include "Shaders\Compiled\SkyBoxVertexShaderVprt.h"
#include "Shaders\Compiled\SkyBoxVertexShaderNoVprt.h"
#include "Shaders\Compiled\SkyBoxGeometryShaderNoVprt.h"

using namespace Neso;
using namespace winrt::Windows::Foundation::Numerics;

SkyboxRenderer::SkyboxRenderer(
    std::shared_ptr<DX::DeviceResources> deviceResources,
    ID3D11ShaderResourceView* skyboxTexture) :
    m_deviceResources(std::move(deviceResources)),
    m_skyboxTexture(skyboxTexture)
{
    CreateDeviceDependentResources();
}

void SkyboxRenderer::SetTexture(ID3D11ShaderResourceView* skyboxTexture)
{
    m_skyboxTexture = skyboxTexture;
}

void SkyboxRenderer::SetViewProjection(
    const float4x4& cameraToViewLeft,
    const float4x4& viewToProjLeft,
    const float4x4& cameraToViewRight,
    const float4x4& viewToProjRight)
{
    float4x4 invViewProjection[2];

    fail_fast_if(!invert(cameraToViewLeft * viewToProjLeft, &invViewProjection[0]));
    fail_fast_if(!invert(cameraToViewRight * viewToProjRight, &invViewProjection[1]));

    invViewProjection[0] = transpose(invViewProjection[0]);
    invViewProjection[1] = transpose(invViewProjection[1]);

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &invViewProjection, 0, 0);
}

void SkyboxRenderer::CreateDeviceDependentResources()
{
    const auto device = m_deviceResources->GetD3DDevice();

    DirectX::ThrowIfFailed(device->CreatePixelShader(
        g_SkyBoxPixelShader, sizeof(g_SkyBoxPixelShader), nullptr, &m_pixelShader));

    const bool useVprt = m_deviceResources->GetDeviceSupportsVprt();
    const void* vertexShader = (useVprt) ? g_SkyBoxVertexShaderVprt : g_SkyBoxVertexShaderNoVprt;
    const size_t vertexShaderSize = (useVprt) ? _countof(g_SkyBoxVertexShaderVprt) : _countof(g_SkyBoxVertexShaderNoVprt);

    if (!useVprt)
    {
        DirectX::ThrowIfFailed(device->CreateGeometryShader(
            g_SkyBoxGeometryShaderNoVprt, sizeof(g_SkyBoxGeometryShaderNoVprt), nullptr, &m_geometryShader));
    }

    DirectX::ThrowIfFailed(device->CreateVertexShader(
        vertexShader, vertexShaderSize, nullptr, &m_vertexShader));

    {
        const CD3D11_SAMPLER_DESC desc{ CD3D11_DEFAULT() };

        DirectX::ThrowIfFailed(device->CreateSamplerState(
            &desc, &m_samplerState));
    }
    {
        static const float4 NdcQuad[] = {
            { -1.0f, +1.0f, 1.0f, 1.0f },
            { +1.0f, +1.0f, 1.0f, 1.0f },
            { +1.0f, -1.0f, 1.0f, 1.0f },

            { -1.0f, +1.0f, 1.0f, 1.0f },
            { +1.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f, -1.0f, 1.0f, 1.0f },
        };

        const CD3D11_BUFFER_DESC desc{
            sizeof(NdcQuad),
            D3D11_BIND_VERTEX_BUFFER
        };

        const D3D11_SUBRESOURCE_DATA data{
            NdcQuad, 0, 0
        };

        DirectX::ThrowIfFailed(device->CreateBuffer(
            &desc, &data, &m_vertexBuffer));
    }
    {
        const CD3D11_BUFFER_DESC desc{
            sizeof(float4x4) * 2,
            D3D11_BIND_CONSTANT_BUFFER
        };

        DirectX::ThrowIfFailed(device->CreateBuffer(
            &desc, nullptr, &m_constantBuffer));
    }
    {
        CD3D11_DEPTH_STENCIL_DESC desc{
            CD3D11_DEFAULT()
        };

        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

        DirectX::ThrowIfFailed(device->CreateDepthStencilState(
            &desc, &m_depthStencilState));
    }

    const D3D11_INPUT_ELEMENT_DESC NdcDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DirectX::ThrowIfFailed(device->CreateInputLayout(
        NdcDesc, _countof(NdcDesc), vertexShader, vertexShaderSize, &m_inputLayout));
}

void SkyboxRenderer::ReleaseDeviceDependentResources()
{
    m_skyboxTexture = nullptr;
    m_vertexShader = nullptr;
    m_geometryShader = nullptr;
    m_pixelShader = nullptr;
    m_samplerState = nullptr;
    m_depthStencilState = nullptr;
    m_vertexBuffer = nullptr;
    m_constantBuffer = nullptr;
    m_inputLayout = nullptr;
}

void SkyboxRenderer::Bind()
{
    const auto context = m_deviceResources->GetD3DDeviceContext();

    context->OMGetDepthStencilState(&m_depthStencilState_Backup, &m_stencilRef_Backup);

    const UINT strides[1] = { sizeof(float4) };
    const UINT offsets[1] = { 0 };
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), strides, offsets);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShaderResources(0, 1, m_skyboxTexture.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}

void SkyboxRenderer::Render()
{
    const auto context = m_deviceResources->GetD3DDeviceContext();

    // Fullscreen Quad: 6 vertices
    context->DrawInstanced(6, 2, 0, 0);
}

void SkyboxRenderer::Unbind()
{
    const auto context = m_deviceResources->GetD3DDeviceContext();

    context->OMSetDepthStencilState(m_depthStencilState_Backup.Get(), m_stencilRef_Backup);

    m_depthStencilState_Backup = nullptr;
    m_stencilRef_Backup = 0;
}


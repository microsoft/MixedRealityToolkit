////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "QuadRenderer.h"
#include "Common\DirectXHelper.h"
#include "DirectXTK\PlatformHelpers.h"

#include "Shaders\Compiled\QuadGeometryShader.h"
#include "Shaders\Compiled\QuadPixelShaderRGB.h"
#include "Shaders\Compiled\QuadVertexShader.h"
#include "Shaders\Compiled\QuadVPRTVertexShader.h"

using namespace Neso;
using namespace Concurrency;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI::Input::Spatial;

struct QuadModelConstantBuffer
{
    winrt::Windows::Foundation::Numerics::float4x4 model;
};

struct QuadRenderingParametersConstantBuffer
{
    winrt::Windows::Foundation::Numerics::float4x4 viewProj[2];
};

// Assert that the constant buffer remains 16-byte aligned (best practice).
static_assert((sizeof(QuadModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");

struct VertexPositionTex
{
    winrt::Windows::Foundation::Numerics::float3 pos;
    winrt::Windows::Foundation::Numerics::float2 tex;
};

// Loads vertex and pixel shaders from files and instantiates the quad geometry.
QuadRenderer::QuadRenderer(std::shared_ptr<DX::DeviceResources> deviceResources) :
    m_deviceResources(std::move(deviceResources))
{
    CreateDeviceDependentResources();
}

void QuadRenderer::Bind()
{
    const auto context = m_deviceResources->GetD3DDeviceContext();

    const UINT strides[] = { sizeof(VertexPositionTex) };
    const UINT offsets[] = { 0 };

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), strides, offsets);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.Get());

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_modelConstantBuffer.GetAddressOf());
    context->VSSetConstantBuffers(1, 1, m_renderingConstantBuffer.GetAddressOf());

    if (!m_deviceResources->GetDeviceSupportsVprt())
    {
        // On devices that do not support the D3D11_FEATURE_D3D11_OPTIONS3::
        // VPAndRTArrayIndexFromAnyShaderFeedingRasterizer optional feature,
        // a pass-through geometry shader sets the render target ID.
        context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
    }

    context->PSSetShader(m_pixelShaderRGB.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}

void QuadRenderer::Render(winrt::Windows::Foundation::Numerics::float4x4 const& matrix, ID3D11ShaderResourceView* texture)
{
    const auto context = m_deviceResources->GetD3DDeviceContext();

    QuadModelConstantBuffer cb;
    cb.model = transpose(matrix);

    context->UpdateSubresource(m_modelConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    context->PSSetShaderResources(0, 1, &texture);

    context->DrawIndexedInstanced(m_indexCount, 2, 0, 0, 0);
}

void QuadRenderer::Unbind()
{}

void QuadRenderer::SetViewProjection(
    const float4x4& worldToViewLeft, 
    const float4x4& viewToProjLeft, 
    const float4x4& worldToViewRight, 
    const float4x4& viewToProjRight)
{
    float4x4 viewProjection[2];

    viewProjection[0] = transpose(worldToViewLeft * viewToProjLeft);
    viewProjection[1] = transpose(worldToViewRight * viewToProjRight);

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_renderingConstantBuffer.Get(), 0, nullptr, &viewProjection, 0, 0);
}

void QuadRenderer::CreateDeviceDependentResources()
{
    const bool usingVprt = m_deviceResources->GetDeviceSupportsVprt();
    const void* vertexShader = (usingVprt) ? g_QuadVPRTVertexShader : g_QuadVertexShader;
    const size_t vertexShaderSize = (usingVprt) ? _countof(g_QuadVPRTVertexShader) : _countof(g_QuadVertexShader);

    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateVertexShader(
            vertexShader,
            vertexShaderSize,
            nullptr,
            &m_vertexShader
        )
    );

    constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 2> vertexDesc = {{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    }};

    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateInputLayout(
            vertexDesc.data(),
            static_cast<UINT>(vertexDesc.size()),
            vertexShader,
            vertexShaderSize,
            &m_inputLayout
        )
    );

    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreatePixelShader(
            g_QuadPixelShaderRGB,
            _countof(g_QuadPixelShaderRGB),
            nullptr,
            &m_pixelShaderRGB
        )
    );

    if (!usingVprt)
    {
        DirectX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateGeometryShader(
                g_QuadGeometryShader,
                _countof(g_QuadGeometryShader),
                nullptr,
                &m_geometryShader
            )
        );
    }

    static const std::array<VertexPositionTex, 4> quadVertices = {{
        { float3(-0.5f, +0.5f, +0.0f), float2(+0.0f, +0.0f) },
        { float3(+0.5f, +0.5f, +0.0f), float2(+1.0f, +0.0f) },
        { float3(+0.5f, -0.5f, +0.0f), float2(+1.0f, +1.0f) },
        { float3(-0.5f, -0.5f, +0.0f), float2(+0.0f, +1.0f) },
    }};

    const D3D11_SUBRESOURCE_DATA vertexBufferData = { quadVertices.data() };

    const CD3D11_BUFFER_DESC vertexBufferDesc{
        static_cast<UINT>(sizeof(VertexPositionTex) * quadVertices.size()),
        D3D11_BIND_VERTEX_BUFFER
    };

    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            &m_vertexBuffer
        )
    );

    constexpr std::array<uint16_t, 12> quadIndices = {{
        // -z
        0,2,3,
        0,1,2,

        // +z
        2,0,3,
        1,0,2,
    }};

    m_indexCount = static_cast<uint32_t>(quadIndices.size());

    const D3D11_SUBRESOURCE_DATA indexBufferData = { quadIndices.data() };

    const CD3D11_BUFFER_DESC indexBufferDesc{
        static_cast<UINT>(sizeof(uint16_t) * quadIndices.size()),
        D3D11_BIND_INDEX_BUFFER
    };
    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            &m_indexBuffer
        )
    );

    const D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());

    DirectX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateSamplerState(
            &desc,
            &m_samplerState
        )
    );
    {
        const CD3D11_BUFFER_DESC constantBufferDesc{
            sizeof(QuadModelConstantBuffer),
            D3D11_BIND_CONSTANT_BUFFER
        };

        DirectX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateBuffer(
                &constantBufferDesc,
                nullptr,
                &m_modelConstantBuffer
            )
        );
    }
    {
        const CD3D11_BUFFER_DESC constantBufferDesc{
            sizeof(float4x4) * 2,
            D3D11_BIND_CONSTANT_BUFFER
        };

        DirectX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateBuffer(
                &constantBufferDesc,
                nullptr,
                &m_renderingConstantBuffer
            )
        );
    }
}

void QuadRenderer::ReleaseDeviceDependentResources()
{
    m_vertexShader.Reset();
    m_inputLayout.Reset();
    m_pixelShaderRGB.Reset();
    m_geometryShader.Reset();

    m_modelConstantBuffer.Reset();
    m_renderingConstantBuffer.Reset();

    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();

    m_samplerState.Reset();
}

#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrMaterial.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Pbr
{
    Material::Material(_In_ ID3D11Device* device)
    {
        const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ConstantBufferData), D3D11_BIND_CONSTANT_BUFFER);
        Internal::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, m_constantBuffer.GetAddressOf()));

        D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
        rtBlendDesc.BlendEnable = TRUE;
        rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
        rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
        rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        CD3D11_BLEND_DESC blendStateDesc(D3D11_DEFAULT);
        for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            blendStateDesc.RenderTarget[i] = rtBlendDesc;
        }

        Internal::ThrowIfFailed(device->CreateBlendState(&blendStateDesc, &m_blendState));
    }

    std::shared_ptr<Material> Material::Clone(_In_ ID3D11Device* device) const
    {
        auto clone = std::make_shared<Material>(device);
        clone->Name = Name;
        clone->Parameters = Parameters;
        clone->Hidden = Hidden;
        clone->m_textures = m_textures;
        clone->m_samplers = m_samplers;
        return clone;
    }

    /* static */
    std::shared_ptr<Material> Material::CreateFlat(_In_ ID3D11Device* device, const Resources& pbrResources, std::string name, CXMVECTOR baseColorFactor, float roughnessFactor /* = 1.0f */, float metallicFactor /* = 0.0f */, CXMVECTOR emissiveFactor /* = XMFLOAT3(0, 0, 0) */)
    {
        std::shared_ptr<Material> material = std::make_shared<Material>(device);
        material->Name = std::move(name);

        material->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
            XMStoreFloat4(&data.BaseColorFactor, baseColorFactor);
            XMStoreFloat3(&data.EmissiveFactor, emissiveFactor);
            data.MetallicFactor = metallicFactor;
            data.RoughnessFactor = roughnessFactor;
        });

        const ComPtr<ID3D11SamplerState> defaultSampler = Pbr::Texture::CreateSampler(device);
        material->SetTexture(ShaderSlots::BaseColor, pbrResources.CreateSolidColorTexture(device, XMVECTORF32{ 1, 1, 1, 1 }).Get(), defaultSampler.Get());
        material->SetTexture(ShaderSlots::MetallicRoughness, pbrResources.CreateSolidColorTexture(device, XMVECTORF32{ 1, 1, 1, 1 }).Get(), defaultSampler.Get());
        material->SetTexture(ShaderSlots::Occlusion, pbrResources.CreateSolidColorTexture(device, XMVECTORF32{ 1, 1, 1, 1 }).Get(), defaultSampler.Get()); // No occlusion.
        material->SetTexture(ShaderSlots::Normal, pbrResources.CreateSolidColorTexture(device, XMVECTORF32{ 0.5f, 0.5f, 1, 1 }).Get(), defaultSampler.Get()); // Flat normal.
        material->SetTexture(ShaderSlots::Emissive, pbrResources.CreateSolidColorTexture(device, XMVECTORF32{ 1, 1, 1, 1 }).Get(), defaultSampler.Get());

        return material;
    }

    void Material::SetTexture(ShaderSlots::PSMaterial slot, _In_ ID3D11ShaderResourceView* textureView, _In_opt_ ID3D11SamplerState* sampler)
    {
        m_textures[slot] = textureView;
        m_samplers[slot] = sampler;
    }

    void Material::Bind(_In_ ID3D11DeviceContext3* context) const
    {
        // If the parameters of the constant buffer have changed, update the constant buffer.
        if (Parameters.UpdateChangeCountBookmark(&m_constantBufferBookmark))
        {
            context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &Parameters.Get(), 0, 0);
        }

        context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFF);
        context->PSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Material, 1, m_constantBuffer.GetAddressOf());

        static_assert(Pbr::ShaderSlots::BaseColor == 0, "BaseColor must be the first slot");
        context->PSSetShaderResources(Pbr::ShaderSlots::BaseColor, (UINT)m_textures.size(), m_textures[0].GetAddressOf());
        context->PSSetSamplers(Pbr::ShaderSlots::BaseColor, (UINT)m_samplers.size(), m_samplers[0].GetAddressOf());
    }
}
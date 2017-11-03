#pragma once

#include <vector>
#include <array>
#include <map>
#include <memory>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "PbrResources.h"

namespace Pbr
{
    // A Material contains the metallic roughness parameters and textures.
    // Primitives specify which Material to use when being rendered.
    struct Material
    {
        // Coefficients used by the shader. Each texture is sampled and multiplied by these coefficients.
        struct ConstantBufferData
        {
            alignas(16) // packoffset(c0)
            DirectX::XMFLOAT4 BaseColorFactor{ 1, 1, 1, 1 };
            alignas(16) // packoffset(c1.x and c1.y)
            float MetallicFactor{ 1 };
            float RoughnessFactor{ 1 };
            alignas(16) // packoffset(c2)
            DirectX::XMFLOAT3 EmissiveFactor{ 1, 1, 1 };
            alignas(16) // packoffset(c3.x and c3.y)
            float NormalScale{ 1 };
            float OcclusionStrength{ 1 };
        };

        static_assert((sizeof(ConstantBufferData) % 16) == 0, "Constant Buffer must be divisible by 16 bytes");

        Material(_In_ ID3D11Device* device);

        std::shared_ptr<Material> Clone(_In_ ID3D11Device* device) const;

        static std::shared_ptr<Material> CreateFlat(
            _In_ ID3D11Device* device,
            const Resources& pbrResources,
            std::string name,
            DirectX::CXMVECTOR baseColorFactor,
            float roughnessFactor = 1.0f,
            float metallicFactor = 0.0f,
            DirectX::CXMVECTOR emissiveFactor = DirectX::Colors::Black);

        void SetTexture(ShaderSlots::PSMaterial slot, _In_ ID3D11ShaderResourceView* textureView, _In_opt_ ID3D11SamplerState* sampler);
        void Bind(_In_ ID3D11DeviceContext3* context) const;

        std::string Name;
        TrackChanges<ConstantBufferData> Parameters;
        bool Hidden{ false };

    private:
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
        std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, ShaderSlots::LastMaterialSlot + 1> m_textures;
        std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>, ShaderSlots::LastMaterialSlot + 1> m_samplers;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
        mutable uint32_t m_constantBufferBookmark{ (uint32_t)-1 };
    };
}
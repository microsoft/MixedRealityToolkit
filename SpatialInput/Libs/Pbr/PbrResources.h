#pragma once

#include <vector>
#include <map>
#include <memory>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <DirectXMath.h>
#include "PbrCommon.h"

namespace Pbr
{
    namespace ShaderSlots
    {
        enum VSResourceViews
        {
            Transforms = 0,
        };

        enum PSMaterial // For both samplers and textures.
        {
            BaseColor = 0,
            MetallicRoughness,
            Normal,
            Occlusion,
            Emissive,
            LastMaterialSlot = Emissive
        };

        enum Pbr // For both samplers and textures.
        {
            Brdf = LastMaterialSlot + 1
        };

        enum EnvironmentMap // For both samplers and textures.
        {
            SpecularTexture = Brdf + 1,
            DiffuseTexture = SpecularTexture + 1,
            EnvironmentMapSampler = Brdf + 1
        };

        enum ConstantBuffers
        {
            Scene,          // Used by VS and PS
            Material,       // PS only
        };
    }

    // Global PBR resources required for rendering a scene.
    struct Resources
    {
        Resources(_In_ ID3D11Device* d3dDevice);

        // Sets the Bidirectional Reflectance Distribution Function Lookup Table texture, required by the shader to compute surface reflectance from the IBL.
        void SetBrdfLut(_In_ ID3D11ShaderResourceView* brdfLut);

        void CreateDeviceDependentResources(_In_ ID3D11Device* device);
        void ReleaseDeviceDependentResources();

        void XM_CALLCONV SetLight(DirectX::XMVECTOR direction, DirectX::XMVECTOR diffuseColor);

        void XM_CALLCONV SetViewProjection(DirectX::FXMMATRIX viewLeft, DirectX::CXMMATRIX viewRight, DirectX::CXMMATRIX projectionLeft, DirectX::CXMMATRIX projectionRight);

        // Many 1x1 pixel colored textures are used in the PBR system. This is used to create textures backed by a cache to reduce the number of textures created.
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSolidColorTexture(_In_ ID3D11Device* device, DirectX::CXMVECTOR color) const;

        void Bind(_In_ ID3D11DeviceContext3* context) const;

        void UpdateConstantBuffer(_In_ ID3D11DeviceContext3* context);

        void SetEnvironmentMap(_In_ ID3D11DeviceContext3* context, _In_ ID3D11ShaderResourceView* specularEnvironmentMap, _In_ ID3D11ShaderResourceView* diffuseEnvironmentMap);

    private:
        struct Impl;
        std::shared_ptr<Impl> m_impl;
    };
}
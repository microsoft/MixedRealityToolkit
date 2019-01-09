////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
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
        explicit Resources(_In_ ID3D11Device* d3dDevice);

        // Sets the Bidirectional Reflectance Distribution Function Lookup Table texture, required by the shader to compute surface reflectance from the IBL.
        void SetBrdfLut(_In_ ID3D11ShaderResourceView* brdfLut);

        // Create device-dependent resources.
        void CreateDeviceDependentResources(_In_ ID3D11Device* device);

        // Release device-dependent resources.
        void ReleaseDeviceDependentResources();

        // Get the D3D11Device that the PBR resources are associated with.
        Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const;

        // Set the directional light. 
        void XM_CALLCONV SetLight(DirectX::XMVECTOR direction, DirectX::XMVECTOR diffuseColor);

        // Set the specular and diffuse image-based lighting (IBL) maps. ShaderResourceViews must be TextureCubes.
        void SetEnvironmentMap(_In_ ID3D11DeviceContext3* context, _In_ ID3D11ShaderResourceView* specularEnvironmentMap, _In_ ID3D11ShaderResourceView* diffuseEnvironmentMap);

        // Set the current view and projection matrices.
        void XM_CALLCONV SetViewProjection(DirectX::FXMMATRIX viewLeft, DirectX::CXMMATRIX viewRight, DirectX::CXMMATRIX projectionLeft, DirectX::CXMMATRIX projectionRight);

        // Many 1x1 pixel colored textures are used in the PBR system. This is used to create textures backed by a cache to reduce the number of textures created.
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSolidColorTexture(DirectX::CXMVECTOR color) const;

        // Bind the the PBR resources to the current context.
        void Bind(_In_ ID3D11DeviceContext3* context) const;

    private:
        struct Impl;
        std::shared_ptr<Impl> m_impl;
    };
}

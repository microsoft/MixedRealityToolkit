////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
//
// Shared data types and functions used throughout the Pbr rendering library.
//

#pragma once

#include <vector>
#include <array>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <DirectXMath.h>

namespace Pbr
{
    namespace Internal
    {
        void ThrowIfFailed(HRESULT hr);
    }

    using NodeIndex_t = uint16_t; // This type must align with the type used in the Pbr shaders.

    constexpr Pbr::NodeIndex_t RootNodeIndex = 0;

    // Vertex structure used by the PBR shaders.
    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT4 Tangent;
        DirectX::XMFLOAT2 TexCoord0;
        NodeIndex_t ModelTransformIndex; // Index into the node transforms

        static const D3D11_INPUT_ELEMENT_DESC s_vertexDesc[5];
    };

    struct PrimitiveBuilder
    {
        std::vector<Pbr::Vertex> Vertices;
        std::vector<uint32_t> Indices;

        PrimitiveBuilder& AddSphere(float diameter, uint32_t tessellation, Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex);
        PrimitiveBuilder& AddCube(float sideLength, Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex);
    };

    namespace Texture
    {
        std::array<uint8_t, 4> XM_CALLCONV CreateRGBA(DirectX::FXMVECTOR color);
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadImage(_In_ ID3D11Device* device, _In_reads_bytes_(size) const uint8_t* fileData, uint32_t fileSize);
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateFlatCubeTexture(_In_ ID3D11Device* device, DirectX::CXMVECTOR color, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTexture(_In_ ID3D11Device* device, _In_reads_bytes_(size) const uint8_t* rgba, uint32_t size, int width, int height, DXGI_FORMAT format);
        Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSampler(_In_ ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_CLAMP);
    }

    // Catch changes to a value of type T for lazy sync with DirectX.
    template <typename T>
    class TrackChanges
    {
    public:
        TrackChanges() = default;
        TrackChanges(TrackChanges<T>&& o) : m_value(std::move(o.m_value)) { }
        TrackChanges(const TrackChanges<T>& o) : m_value(o.Get()) { }

        TrackChanges<T>& operator=(TrackChanges<T>&& o)
        {
            InterlockedIncrement(&m_changeCount);
            m_value = std::move(o.m_value);
            return *this;
        }

        TrackChanges<T>& operator=(const TrackChanges<T>& o)
        {
            InterlockedIncrement(&m_changeCount);
            m_value = o.m_value;
            return *this;
        }

        template <typename Func>
        void Set(Func func)
        {
            InterlockedIncrement(&m_changeCount);
            func(m_value);
        }

        bool UpdateChangeCountBookmark(_In_ uint32_t* changeCountBookmark) const
        {
            return InterlockedExchange(changeCountBookmark, m_changeCount) != *changeCountBookmark;
        }

        const T& Get() const { return m_value; }

    private:
        T m_value;
        uint32_t m_changeCount{ 0 };
    };
}

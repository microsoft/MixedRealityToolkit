////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrPrimitive.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace
{
    ComPtr<ID3D11Buffer> CreateVertexBuffer(_In_ ID3D11Device* device, const Pbr::PrimitiveBuilder& primitiveBuilder)
    {
        // Create Vertex Buffer
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = (UINT)(sizeof(decltype(primitiveBuilder.Vertices)::value_type) * primitiveBuilder.Vertices.size());
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = primitiveBuilder.Vertices.data();

        ComPtr<ID3D11Buffer> vertexBuffer;
        Pbr::Internal::ThrowIfFailed(device->CreateBuffer(&desc, &initData, &vertexBuffer));
        return vertexBuffer;
    }

    ComPtr<ID3D11Buffer> CreateIndexBuffer(_In_ ID3D11Device* device, const Pbr::PrimitiveBuilder& primitiveBuilder)
    {
        // Create Index Buffer
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = (UINT)(sizeof(decltype(primitiveBuilder.Indices)::value_type) * primitiveBuilder.Indices.size());
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = primitiveBuilder.Indices.data();

        ComPtr<ID3D11Buffer> indexBuffer;
        Pbr::Internal::ThrowIfFailed(device->CreateBuffer(&desc, &initData, &indexBuffer));
        return indexBuffer;
    }
}

namespace Pbr
{
    Primitive::Primitive(UINT indexCount, ID3D11Buffer* indexBuffer, ID3D11Buffer* vertexBuffer, std::shared_ptr<Material> material)
        : m_indexCount(indexCount)
        , m_vertexBuffer(vertexBuffer)
        , m_indexBuffer(indexBuffer)
        , m_material(std::move(material))
    {
    }

    Primitive::Primitive(Pbr::Resources const& pbrResources, const Pbr::PrimitiveBuilder& primitiveBuilder, std::shared_ptr<Pbr::Material> material)
        : Primitive(
            (UINT)primitiveBuilder.Indices.size(),
            CreateIndexBuffer(pbrResources.GetDevice().Get(), primitiveBuilder).Get(),
            CreateVertexBuffer(pbrResources.GetDevice().Get(), primitiveBuilder).Get(),
            std::move(material))
    {
    }

    Primitive Primitive::Clone(Pbr::Resources const& pbrResources) const
    {
        return Primitive(m_indexCount, m_indexBuffer.Get(), m_vertexBuffer.Get(), m_material->Clone(pbrResources));
    }

    void Primitive::Render(_In_ ID3D11DeviceContext3* context) const
    {
        const UINT stride = sizeof(Pbr::Vertex);
        const UINT offset = 0;
        context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexedInstanced(m_indexCount, 2, 0, 0, 0);
    }
}

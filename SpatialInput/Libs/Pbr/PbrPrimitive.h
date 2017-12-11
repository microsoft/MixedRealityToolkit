////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <vector>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include "PbrMaterial.h"

namespace Pbr
{
    // A primitive holds a vertex buffer, index buffer, and a pointer to a PBR material.
    struct Primitive
    {
        using Collection = std::vector<Primitive>;

        Primitive() = delete;
        Primitive(UINT indexCount, _In_ ID3D11Buffer* indexBuffer, _In_ ID3D11Buffer* vertexBuffer, std::shared_ptr<Material> material);
        Primitive(Pbr::Resources const& pbrResources, const Pbr::PrimitiveBuilder& primitiveBuilder, std::shared_ptr<Material> material);

        // Get the material for the primitive.
        std::shared_ptr<Material>& GetMaterial() { return m_material; }
        const std::shared_ptr<Material>& GetMaterial() const { return m_material; }

    protected:
        friend struct Model;
        void Render(_In_ ID3D11DeviceContext3* context) const;
        Primitive Clone(Pbr::Resources const& pbrResources) const;

    private:
        UINT m_indexCount;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        std::shared_ptr<Material> m_material;
    };
}

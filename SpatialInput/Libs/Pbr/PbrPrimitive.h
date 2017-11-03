#pragma once

#include <vector>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include "PbrMaterial.h"

namespace Pbr
{
    struct Primitive
    {
        using Collection = std::vector<Primitive>;

        Primitive() = delete;
        Primitive(UINT indexCount, _In_ ID3D11Buffer* indexBuffer, _In_ ID3D11Buffer* vertexBuffer, std::shared_ptr<Material> material);
        Primitive(_In_ ID3D11Device* device, const Pbr::PrimitiveBuilder& primitiveBuilder, std::shared_ptr<Material> material);

        void Render(_In_ ID3D11DeviceContext3* context) const;

        Primitive Clone(_In_ ID3D11Device* device) const;

        std::shared_ptr<Material>& GetMaterial() { return m_material; }
        const std::shared_ptr<Material>& GetMaterial() const { return m_material; }

    private:
        UINT m_indexCount;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        std::shared_ptr<Material> m_material;
    };
}
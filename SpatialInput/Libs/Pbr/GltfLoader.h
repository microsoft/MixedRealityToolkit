//
// Functions to load glTF 2.0 content into a renderable Pbr::Model.
//

#pragma once

#include <memory>
#include "PbrResources.h"
#include "PbrModel.h"

interface ID3D11Device;
interface ID3D11DeviceContext3;

namespace tinygltf { class Model; }

namespace Gltf
{
    // Creates a Pbr Model from tinygltf model.
    std::shared_ptr<Pbr::Model> FromGltfObject(
        _In_ ID3D11Device* device,
        const Pbr::Resources& pbrResources,
        const tinygltf::Model& gltfModel,
        DirectX::CXMMATRIX rootTransform = DirectX::XMMatrixIdentity());

    // Creates a Pbr Model from glTF 2.0 GLB file content.
    std::shared_ptr<Pbr::Model> FromGltfBinary(
        _In_ ID3D11Device* device,
        const Pbr::Resources& pbrResources,
        _In_reads_bytes_(bufferBytes) const byte* buffer,
        uint32_t bufferBytes,
        DirectX::CXMMATRIX rootTransform = DirectX::XMMatrixIdentity());
}
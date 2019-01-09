////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
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
    // Creates a Pbr Model from tinygltf model with the specified root transform.
    std::shared_ptr<Pbr::Model> FromGltfObject(
        const Pbr::Resources& pbrResources,
        const tinygltf::Model& gltfModel,
        DirectX::CXMMATRIX rootTransform);

    // Creates a Pbr Model from tinygltf model.
    inline std::shared_ptr<Pbr::Model> FromGltfObject(
        const Pbr::Resources& pbrResources,
        const tinygltf::Model& gltfModel)
    {
        return FromGltfObject(pbrResources, gltfModel, DirectX::XMMatrixIdentity());
    }

    // Creates a Pbr Model from glTF 2.0 GLB file content with the specified root transform.
    std::shared_ptr<Pbr::Model> FromGltfBinary(
        const Pbr::Resources& pbrResources,
        _In_reads_bytes_(bufferBytes) const byte* buffer,
        uint32_t bufferBytes,
        DirectX::CXMMATRIX rootTransform);

    // Creates a Pbr Model from glTF 2.0 GLB file content.
    inline std::shared_ptr<Pbr::Model> FromGltfBinary(
        const Pbr::Resources& pbrResources,
        _In_reads_bytes_(bufferBytes) const byte* buffer,
        uint32_t bufferBytes)
    {
        return FromGltfBinary(pbrResources, buffer, bufferBytes, DirectX::XMMatrixIdentity());
    }
}

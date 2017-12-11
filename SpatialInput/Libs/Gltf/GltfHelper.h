////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
// GltfHelper provides additional glTF parsing functionality, built on top of tinygltf.
// This library has no rendering dependencies and can be used for any purpose, such as
// format transcoding or by a rendering engine.

#pragma once

#include <DirectXMath.h>
#include <vector>

namespace tinygltf
{
    class Node;
    class Model;
    struct Primitive;
    struct Material;
    struct Image;
    struct Sampler;
}

namespace GltfHelper
{
    // Vertex data.
    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT4 Tangent;
        DirectX::XMFLOAT2 TexCoord0;
        // Note: This implementation does not currently support glTF 2's Color0 and TexCoord1 attributes.
    };

    // A primitive is a collection of vertices and indices.
    struct Primitive
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    // Metallic-roughness material definition.
    struct Material
    {
        struct Texture
        {
            const tinygltf::Image* Image;
            const tinygltf::Sampler* Sampler;
        };

        Texture BaseColorTexture;
        Texture MetallicRoughnessTexture;
        Texture EmissiveTexture;
        Texture NormalTexture;
        Texture OcclusionTexture;

        DirectX::XMFLOAT4 BaseColorFactor;
        float MetallicFactor;
        float RoughnessFactor;
        DirectX::XMFLOAT3 EmissiveFactor;

        float NormalScale;
        float OcclusionStrength;
    };

    // Reads the "transform" or "TRS" data for a Node as an XMMATRIX.
    DirectX::XMMATRIX XM_CALLCONV ReadNodeLocalTransform(const tinygltf::Node& gltfNode);

    // Parses the primitive attributes and indices from the glTF accessors/bufferviews/buffers into a common simplified data structure, the Primitive.
    Primitive ReadPrimitive(const tinygltf::Model& gltfModel, const tinygltf::Primitive& gltfPrimitive);

    // Parses the material values into a simplified data structure, the Material.
    Material ReadMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial);

    // Converts the image to RGBA if necessary. Requires a temporary buffer only if it needs to be converted.
    const uint8_t* ReadImageAsRGBA(const tinygltf::Image& image, _Inout_ std::vector<uint8_t>* tempBuffer);
}

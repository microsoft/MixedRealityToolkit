////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include <tiny_gltf.h>
#include <mikktspace.h>
#include "GltfHelper.h"

using namespace DirectX;

#define TRIANGLE_VERTEX_COUNT 3 // #define so it can be used in lambdas without capture

namespace
{
    // The glTF 2 specification recommends using the MikkTSpace algorithm to generate
    // tangents when none are available. This function takes a GltfHelper Primitive which has
    // no tangents and uses the MikkTSpace algorithm to generate the tangents. This can
    // be computationally expensive.
    void ComputeTriangleTangents(GltfHelper::Primitive& primitive)
    {
        // Set up the callbacks so that MikkTSpace can read the Primitive data.
        SMikkTSpaceInterface mikkInterface{};
        mikkInterface.m_getNumFaces = [](const SMikkTSpaceContext* pContext) {
            auto primitive = static_cast<const GltfHelper::Primitive*>(pContext->m_pUserData);
            assert((primitive->Indices.size() % TRIANGLE_VERTEX_COUNT) == 0); // Only triangles are supported.
            return (int)(primitive->Indices.size() / TRIANGLE_VERTEX_COUNT);
        };
        mikkInterface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* pContext, int iFace) {
            return TRIANGLE_VERTEX_COUNT;
        };
        mikkInterface.m_getPosition = [](const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert) {
            auto primitive = static_cast<const GltfHelper::Primitive*>(pContext->m_pUserData);
            const auto vertexIndex = primitive->Indices[(iFace * TRIANGLE_VERTEX_COUNT) + iVert];
            memcpy(fvPosOut, &primitive->Vertices[vertexIndex].Position, sizeof(float) * 3);
        };
        mikkInterface.m_getNormal = [](const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert) {
            auto primitive = static_cast<const GltfHelper::Primitive*>(pContext->m_pUserData);
            const auto vertexIndex = primitive->Indices[(iFace * TRIANGLE_VERTEX_COUNT) + iVert];
            memcpy(fvNormOut, &primitive->Vertices[vertexIndex].Normal, sizeof(float) * 3);
        };
        mikkInterface.m_getTexCoord = [](const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert) {
            auto primitive = static_cast<const GltfHelper::Primitive*>(pContext->m_pUserData);
            const auto vertexIndex = primitive->Indices[(iFace * TRIANGLE_VERTEX_COUNT) + iVert];
            memcpy(fvTexcOut, &primitive->Vertices[vertexIndex].TexCoord0, sizeof(float) * 2);
        };
        mikkInterface.m_setTSpaceBasic = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
            auto primitive = static_cast<GltfHelper::Primitive*>(pContext->m_pUserData);
            const auto vertexIndex = primitive->Indices[(iFace * TRIANGLE_VERTEX_COUNT) + iVert];
            primitive->Vertices[vertexIndex].Tangent.x = fvTangent[0];
            primitive->Vertices[vertexIndex].Tangent.y = fvTangent[1];
            primitive->Vertices[vertexIndex].Tangent.z = fvTangent[2];
            primitive->Vertices[vertexIndex].Tangent.w = fSign;
        };

        // Run the MikkTSpace algorithm.
        SMikkTSpaceContext mikkContext{};
        mikkContext.m_pUserData = &primitive;
        mikkContext.m_pInterface = &mikkInterface;
        if (genTangSpaceDefault(&mikkContext) == 0)
        {
            throw std::exception("Failed to generate tangents");
        }
    }

    // Generates normals for the trianges in the GltfHelper Primitive object.
    void ComputeTriangleNormals(GltfHelper::Primitive& primitive)
    {
        assert((primitive.Indices.size() % TRIANGLE_VERTEX_COUNT) == 0); // Only triangles are supported.

        // Loop through each triangle
        for (uint32_t i = 0; i < primitive.Indices.size(); i += TRIANGLE_VERTEX_COUNT)
        {
            // References to the three vertices of the triangle.
            GltfHelper::Vertex& v0 = primitive.Vertices[primitive.Indices[i]];
            GltfHelper::Vertex& v1 = primitive.Vertices[primitive.Indices[i + 1]];
            GltfHelper::Vertex& v2 = primitive.Vertices[primitive.Indices[i + 2]];

            // Compute normal. Normalization happens later.
            const XMVECTOR pos0 = XMLoadFloat3(&v0.Position);
            const XMVECTOR d0 = XMVectorSubtract(XMLoadFloat3(&v2.Position), pos0);
            const XMVECTOR d1 = XMVectorSubtract(XMLoadFloat3(&v1.Position), pos0);
            const XMVECTOR normal = XMVector3Cross(d0, d1);

            // Add the normal to the three vertices of the triangle. Normals are added
            // so that reused vertices will get the average normal (done later).
            // Note that the normals are not normalized at this point, so larger triangles
            // will have more weight than small triangles which share a vertex. This
            // appears to give better results.
            XMStoreFloat3(&v0.Normal, XMVectorAdd(XMLoadFloat3(&v0.Normal), normal));
            XMStoreFloat3(&v1.Normal, XMVectorAdd(XMLoadFloat3(&v1.Normal), normal));
            XMStoreFloat3(&v2.Normal, XMVectorAdd(XMLoadFloat3(&v2.Normal), normal));
        }

        // Since the same vertex may have been used by multiple triangles, and the cross product normals
        // aren't normalized yet, normalize the computed normals.
        for (GltfHelper::Vertex& vertex : primitive.Vertices)
        {
            XMStoreFloat3(&vertex.Normal, XMVector3Normalize(XMLoadFloat3(&vertex.Normal)));
        }
    }

    // Some data, like texCoords, can be represented 32bit float or normalized unsigned short or byte.
    // ReadNormalizedFloat provides overloads for all three types.
    template <typename T> float ReadNormalizedFloat(const uint8_t* ptr);
    template<> float ReadNormalizedFloat<float>(const uint8_t* ptr) { return *reinterpret_cast<const float*>(ptr); }
    template<> float ReadNormalizedFloat<unsigned short>(const uint8_t* ptr) { return *reinterpret_cast<const unsigned short*>(ptr) / (float)std::numeric_limits<unsigned short>::max(); }
    template<> float ReadNormalizedFloat<unsigned char>(const uint8_t* ptr) { return *reinterpret_cast<const unsigned char*>(ptr) / (float)std::numeric_limits<unsigned char>::max(); }

    // Convert array of 16 doubles to an XMMATRIX.
    XMMATRIX XM_CALLCONV Double4x4ToXMMatrix(FXMMATRIX defaultMatrix, const std::vector<double>& doubleData)
    {
        if (doubleData.size() != 16)
        {
            return defaultMatrix;
        }

        return XMMATRIX(
            (float)doubleData[0], (float)doubleData[1], (float)doubleData[2], (float)doubleData[3],
            (float)doubleData[4], (float)doubleData[5], (float)doubleData[6], (float)doubleData[7],
            (float)doubleData[8], (float)doubleData[9], (float)doubleData[10], (float)doubleData[11],
            (float)doubleData[12], (float)doubleData[13], (float)doubleData[14], (float)doubleData[15]);
    }

    // Convert array of three doubles to an XMVECTOR.
    XMVECTOR XM_CALLCONV Double3ToXMVector(FXMVECTOR defaultVector, const std::vector<double>& doubleData)
    {
        if (doubleData.size() != 3)
        {
            return defaultVector;
        }

        XMFLOAT3 vec3((float)doubleData[0], (float)doubleData[1], (float)doubleData[2]);
        return XMLoadFloat3(&vec3);
    }

    // Convert array of four doubles to an XMVECTOR.
    XMVECTOR Double4ToXMVector(FXMVECTOR defaultVector, const std::vector<double>& doubleData)
    {
        if (doubleData.size() != 4)
        {
            return defaultVector;
        }

        XMFLOAT4 vec4((float)doubleData[0], (float)doubleData[1], (float)doubleData[2], (float)doubleData[3]);
        return XMLoadFloat4(&vec4);
    }

    // Validate that an accessor does not go out of bounds of the buffer view that it references and that the buffer view does not exceed
    // the bounds of the buffer that it references.
    void ValidateAccessor(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, size_t byteStride, size_t elementSize)
    {
        // Make sure the accessor does not go out of range of the buffer view.
        if (accessor.byteOffset + (accessor.count - 1) * byteStride + elementSize > bufferView.byteLength)
        {
            throw std::out_of_range("Accessor goes out of range of bufferview.");
        }

        // Make sure the buffer view does not go out of range of the buffer.
        if (bufferView.byteOffset + bufferView.byteLength > buffer.data.size())
        {
            throw std::out_of_range("BufferView goes out of range of buffer.");
        }
    }

    // Reads the tangent data (VEC4) from a glTF primitive into a GltfHelper Primitive.
    void XM_CALLCONV ReadTangentToVertexField(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, GltfHelper::Primitive& primitive)
    {
        if (accessor.type != TINYGLTF_TYPE_VEC4)
        {
            throw std::exception("Accessor for primitive attribute has incorrect type (VEC4 expected).");
        }

        if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            throw std::exception("Accessor for primitive attribute has incorrect component type (FLOAT expected).");
        }

        // If stride is not specified, it is tightly packed.
        constexpr size_t PackedSize = sizeof(XMFLOAT4);
        const size_t stride = bufferView.byteStride == 0 ? PackedSize : bufferView.byteStride;
        ValidateAccessor(accessor, bufferView, buffer, stride, PackedSize);

        // Resize the vertices vector, if necessary, to include room for the attribute data.
        // If there are multiple attributes for a primitive, the first one will resize, and the subsequent will not need to.
        primitive.Vertices.resize(accessor.count);

        // Copy the attribute value over from the glTF buffer into the appropriate vertex field.
        const unsigned char* bufferPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
        for (size_t i = 0; i < accessor.count; i++, bufferPtr += stride)
        {
            primitive.Vertices[i].Tangent = *reinterpret_cast<const XMFLOAT4*>(bufferPtr);
        }
    }

    // Reads the TexCoord data (VEC2) from a glTF primitive into a GltfHelper Primitive.
    // This function uses a template type to express the VEC2 component type (byte, ushort, or float).
    template <typename TComponentType, XMFLOAT2 GltfHelper::Vertex::*field>
    void ReadTexCoordToVertexField(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, GltfHelper::Primitive& primitive)
    {
        // If stride is not specified, it is tightly packed.
        constexpr size_t PackedSize = sizeof(TComponentType) * 2;
        const size_t stride = bufferView.byteStride == 0 ? PackedSize : bufferView.byteStride;
        ValidateAccessor(accessor, bufferView, buffer, stride, PackedSize);

        // Resize the vertices vector, if necessary, to include room for the attribute data.
        // If there are multiple attributes for a primitive, the first one will resize, and the subsequent will not need to.
        primitive.Vertices.resize(accessor.count);

        // Copy the attribute value over from the glTF buffer into the appropriate vertex field.
        const unsigned char* bufferPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
        for (size_t i = 0; i < accessor.count; i++, bufferPtr += stride)
        {
            (primitive.Vertices[i].*field).x = ReadNormalizedFloat<TComponentType>(bufferPtr);
            (primitive.Vertices[i].*field).y = ReadNormalizedFloat<TComponentType>(bufferPtr + sizeof(TComponentType));
        }
    }

    // Reads the TexCoord data (VEC2) from a glTF primitive into a GltfHelper Primitive.
    template <XMFLOAT2 GltfHelper::Vertex::*field>
    void ReadTexCoordToVertexField(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, GltfHelper::Primitive& primitive)
    {
        if (accessor.type != TINYGLTF_TYPE_VEC2)
        {
            throw std::exception("Accessor for primitive TexCoord must have VEC2 type.");
        }

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            ReadTexCoordToVertexField<float, field>(accessor, bufferView, buffer, primitive);
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            if (!accessor.normalized) { throw std::exception("Accessor for TEXTCOORD_n unsigned byte must be normalized."); }
            ReadTexCoordToVertexField<uint8_t, field>(accessor, bufferView, buffer, primitive);
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            if (!accessor.normalized) { throw std::exception("Accessor for TEXTCOORD_n unsigned short must be normalized."); }
            ReadTexCoordToVertexField<unsigned short, field>(accessor, bufferView, buffer, primitive);
        }
        else
        {
            throw std::exception("Accessor for TEXTCOORD_n uses unsupported component type.");
        }
    }

    // Reads VEC3 attribute data (like POSITION and NORMAL) from a glTF primitive into a GltfHelper Primitive. The specific Vertex field is specified as a template parameter.
    template <XMFLOAT3 GltfHelper::Vertex::*field>
    void XM_CALLCONV ReadVec3ToVertexField(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, GltfHelper::Primitive& primitive)
    {
        if (accessor.type != TINYGLTF_TYPE_VEC3)
        {
            throw std::exception("Accessor for primitive attribute has incorrect type (VEC3 expected).");
        }

        if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            throw std::exception("Accessor for primitive attribute has incorrect component type (FLOAT expected).");
        }

        // If stride is not specified, it is tightly packed.
        constexpr size_t PackedSize = sizeof(XMFLOAT3);
        const size_t stride = bufferView.byteStride == 0 ? PackedSize : bufferView.byteStride;
        ValidateAccessor(accessor, bufferView, buffer, stride, PackedSize);

        // Resize the vertices vector, if necessary, to include room for the attribute data.
        // If there are multiple attributes for a primitive, the first one will resize, and the subsequent will not need to.
        primitive.Vertices.resize(accessor.count);

        // Copy the attribute value over from the glTF buffer into the appropriate vertex field.
        const unsigned char* bufferPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
        for (size_t i = 0; i < accessor.count; i++, bufferPtr += stride)
        {
            (primitive.Vertices[i].*field) = *reinterpret_cast<const XMFLOAT3*>(bufferPtr);
        }
    }

    // Load a primitive's (vertex) attributes. Vertex attributes can be positions, normals, tangents, texture coordinates, colors, and more.
    void XM_CALLCONV LoadAttributeAccessor(const tinygltf::Model& gltfModel, const std::string& attributeName, int accessorId, GltfHelper::Primitive& primitive)
    {
        const auto& accessor = gltfModel.accessors.at(accessorId);

        if (accessor.bufferView == -1)
        {
            throw std::exception("Accessor for primitive attribute specifies no bufferview.");
        }

        // WARNING: This version of the tinygltf loader does not support sparse accessors, so neither does this renderer.

        const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(accessor.bufferView);
        if (bufferView.target != TINYGLTF_TARGET_ARRAY_BUFFER && bufferView.target != 0)  // Allow 0 (not specified) even though spec doesn't seem to allow this (BoomBox GLB fails)
        {
            throw std::exception("Accessor for primitive attribute uses bufferview with invalid 'target' type.");
        }

        const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);

        if (attributeName.compare("POSITION") == 0)
        {
            ReadVec3ToVertexField<&GltfHelper::Vertex::Position>(accessor, bufferView, buffer, primitive);
        }
        else if (attributeName.compare("NORMAL") == 0)
        {
            ReadVec3ToVertexField<&GltfHelper::Vertex::Normal>(accessor, bufferView, buffer, primitive);
        }
        else if (attributeName.compare("TANGENT") == 0)
        {
            ReadTangentToVertexField(accessor, bufferView, buffer, primitive);
        }
        else if (attributeName.compare("TEXCOORD_0") == 0)
        {
            ReadTexCoordToVertexField<&GltfHelper::Vertex::TexCoord0>(accessor, bufferView, buffer, primitive);
        }
        else
        {
            return; // Ignore unsupported vertex accessors like COLOR_0.
        }
    }

    // Reads index data from a glTF primitive into a GltfHelper Primitive. glTF indices may be 8bit, 16bit or 32bit integers.
    // This will coalesce indices from the source type(s) into a 32bit integer.
    template <typename TSrcIndex>
    void ReadIndices(const tinygltf::Accessor& accessor, const tinygltf::BufferView& bufferView, const tinygltf::Buffer& buffer, GltfHelper::Primitive& primitive)
    {
        if (bufferView.target != TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER && bufferView.target != 0) // Allow 0 (not specified) even though spec doesn't seem to allow this (BoomBox GLB fails)
        {
            throw std::exception("Accessor for indices uses bufferview with invalid 'target' type.");
        }

        constexpr size_t ComponentSizeBytes = sizeof(TSrcIndex);
        if (bufferView.byteStride != 0 && bufferView.byteStride != ComponentSizeBytes) // Index buffer must be packed per glTF spec.
        {
            throw std::exception("Accessor for indices uses bufferview with invalid 'byteStride'.");
        }

        ValidateAccessor(accessor, bufferView, buffer, ComponentSizeBytes, ComponentSizeBytes);

        if ((accessor.count % 3) != 0) // Since only triangles are supported, enforce that the number of indices is divisible by 3.
        {
            throw std::exception("Unexpected number of indices for triangle primitive");
        }

        const TSrcIndex* indexBuffer = reinterpret_cast<const TSrcIndex*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
        for (uint32_t i = 0; i < accessor.count; i++)
        {
            primitive.Indices.push_back(*(indexBuffer + i));
        }
    }

    // Reads index data from a glTF primitive into a GltfHelper Primitive.
    void LoadIndexAccessor(const tinygltf::Model& gltfModel, const tinygltf::Accessor& accessor, GltfHelper::Primitive& primitive)
    {
        if (accessor.type != TINYGLTF_TYPE_SCALAR)
        {
            throw std::exception("Accessor for indices specifies invalid 'type'.");
        }

        if (accessor.bufferView == -1)
        {
            throw std::exception("Index accessor without bufferView is currently not supported.");
        }

        const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            ReadIndices<unsigned char>(accessor, bufferView, buffer, primitive);
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            ReadIndices<unsigned short>(accessor, bufferView, buffer, primitive);
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        {
            ReadIndices<unsigned int>(accessor, bufferView, buffer, primitive);
        }
        else
        {
            throw std::exception("Accessor for indices specifies invalid 'componentType'.");
        }
    }
}

namespace GltfHelper
{
    XMMATRIX XM_CALLCONV ReadNodeLocalTransform(const tinygltf::Node& gltfNode)
    {
        // A node may specify either a 4x4 matrix or TRS (Translation-Rotation-Scale) values, but not both.
        if (gltfNode.matrix.size() == 16)
        {
            return Double4x4ToXMMatrix(XMMatrixIdentity(), gltfNode.matrix);
        }
        else
        {
            // No matrix is present, so construct a matrix from the TRS values (each one is optional).
            return XMMatrixTransformation(
                g_XMZero, // Scaling origin
                XMQuaternionIdentity(),
                Double3ToXMVector(g_XMOne, gltfNode.scale), // Scale
                g_XMZero, // Rotating origin
                Double4ToXMVector(XMQuaternionIdentity(), gltfNode.rotation), // Rotation
                Double3ToXMVector(g_XMZero, gltfNode.translation)); // Translation
        }
    }

    Primitive ReadPrimitive(const tinygltf::Model& gltfModel, const tinygltf::Primitive& gltfPrimitive)
    {
        if (gltfPrimitive.mode != TINYGLTF_MODE_TRIANGLES)
        {
            throw std::exception("Unsupported primitive mode. Only TINYGLTF_MODE_TRIANGLES is supported.");
        }

        Primitive primitive;

        // glTF vertex data is stored in an attribute dictionary. Loop through each attribute and insert it into the GltfHelper primitive.
        for (const auto& attribute : gltfPrimitive.attributes)
        {
            LoadAttributeAccessor(gltfModel, attribute.first /* attribute name */, attribute.second /* accessor index */, primitive);
        }

        if (gltfPrimitive.indices != -1)
        {
            // If indices are specified for the glTF primitive, read them into the GltfHelper Primitive.
            LoadIndexAccessor(gltfModel, gltfModel.accessors.at(gltfPrimitive.indices), primitive);
        }
        else
        {
            // When indices is not defined, the primitives should be rendered without indices using drawArrays()
            // This is the equivalent to having an index in sequence for each vertex.
            const uint32_t vertexCount = (uint32_t)primitive.Vertices.size();
            if ((vertexCount % 3) != 0)
            {
                throw std::exception("Non-indexed triangle-based primitive must have number of vertices divisible by 3.");
            }

            primitive.Indices.reserve(primitive.Indices.size() + vertexCount);
            for (uint32_t i = 0; i < vertexCount; i ++)
            {
                primitive.Indices.push_back(i);
            }
        }

        // If normals are missing, compute flat normals. Normals must be computed before tangents.
        if (gltfPrimitive.attributes.find("NORMAL") == std::end(gltfPrimitive.attributes))
        {
            ComputeTriangleNormals(primitive);
        }

        // If tangents are missing, compute tangents.
        if (gltfPrimitive.attributes.find("TANGENT") == std::end(gltfPrimitive.attributes))
        {
            ComputeTriangleTangents(primitive);
        }

        return primitive;
    }

    Material ReadMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial)
    {
        // Read an optional VEC4 parameter if available, otherwise use the default.
        auto readParameterFactorAsVec4 = [](const tinygltf::ParameterMap& parameters, char* name, const XMFLOAT4& default) {
            auto c = parameters.find(name);
            return (c != parameters.end() && c->second.number_array.size() == 4) ?
                XMFLOAT4((float)c->second.number_array[0], (float)c->second.number_array[1], (float)c->second.number_array[2], (float)c->second.number_array[3]) :
                default;
        };

        // Read an optional VEC3 parameter if available, otherwise use the default.
        auto readParameterFactorAsVec3 = [](const tinygltf::ParameterMap& parameters, char* name, const XMFLOAT3& default) {
            auto c = parameters.find(name);
            return (c != parameters.end() && c->second.number_array.size() == 3) ?
                XMFLOAT3((float)c->second.number_array[0], (float)c->second.number_array[1], (float)c->second.number_array[2]) :
                default;
        };

        // Read an optional scalar parameter if available, otherwise use the default.
        auto readParameterFactorAsScalar = [](const tinygltf::ParameterMap& parameters, char* name, double default) {
            auto c = parameters.find(name);
            return (c != parameters.end() && c->second.number_array.size() == 1) ? c->second.number_array[0] : default;
        };

        // Read a specific texture from a tinygltf material parameter map.
        auto loadTextureFromParameter = [&](const tinygltf::ParameterMap& parameterMap, char* textureName)
        {
            Material::Texture texture{};

            const auto& textureIt = parameterMap.find(textureName);
            if (textureIt != std::end(parameterMap))
            {
                const int textureIndex = (int)textureIt->second.json_double_value.at("index");
                const tinygltf::Texture& gltfTexture = gltfModel.textures.at(textureIndex);
                if (gltfTexture.source != -1)
                {
                    texture.Image = &gltfModel.images.at(gltfTexture.source);
                }

                if (gltfTexture.sampler != -1)
                {
                    texture.Sampler = &gltfModel.samplers.at(gltfTexture.sampler);
                }
            }

            return texture;
        };

        // Read a scalar value from a tinygltf material parameter map.
        auto loadScalarFromParameter = [&](const tinygltf::ParameterMap& parameterMap, char* name, char* scalarField, double defaultValue)
        {
            const auto& textureIt = parameterMap.find(name);
            if (textureIt != std::end(parameterMap))
            {
                const auto& jsonDoubleValues = textureIt->second.json_double_value;
                const auto& jsonDoubleIt = jsonDoubleValues.find(scalarField);
                if (jsonDoubleIt != std::end(jsonDoubleValues))
                {
                    return jsonDoubleIt->second;
                }
            }

            return defaultValue;
        };

        //
        // Read all of the optional material fields from the tinygltf object model and store them in a GltfHelper Material object
        // coalesced with proper defaults when needed.
        //
        Material material;

        material.BaseColorTexture = loadTextureFromParameter(gltfMaterial.values, "baseColorTexture");
        material.BaseColorFactor = readParameterFactorAsVec4(gltfMaterial.values, "baseColorFactor", XMFLOAT4(1, 1, 1, 1));

        material.MetallicRoughnessTexture = loadTextureFromParameter(gltfMaterial.values, "metallicRoughnessTexture");
        material.MetallicFactor = (float)readParameterFactorAsScalar(gltfMaterial.values, "metallicFactor", 1);
        material.RoughnessFactor = (float)readParameterFactorAsScalar(gltfMaterial.values, "roughnessFactor", 1);

        material.EmissiveTexture = loadTextureFromParameter(gltfMaterial.additionalValues, "emissiveTexture");
        material.EmissiveFactor = readParameterFactorAsVec3(gltfMaterial.additionalValues, "emissiveFactor", XMFLOAT3(0, 0, 0));

        material.NormalTexture = loadTextureFromParameter(gltfMaterial.additionalValues, "normalTexture");
        material.NormalScale = (float)loadScalarFromParameter(gltfMaterial.additionalValues, "normalTexture", "scale", 1.0);

        material.OcclusionTexture = loadTextureFromParameter(gltfMaterial.additionalValues, "occlusionTexture");
        material.OcclusionStrength = (float)loadScalarFromParameter(gltfMaterial.additionalValues, "occlusionTexture", "strength", 1.0);

        return material;
    }

    const uint8_t* ReadImageAsRGBA(const tinygltf::Image& image, _Inout_ std::vector<uint8_t>* tempBuffer)
    {
        // The image vector (image.image) will be populated if the image was successfully loaded by glTF.
        if (image.width > 0 && image.height > 0)
        {
            if (image.width * image.height * image.component != image.image.size())
            {
                throw std::exception("Invalid image buffer size");
            }

            // Not supported: STBI_grey (DXGI_FORMAT_R8_UNORM?) and STBI_grey_alpha.
            if (image.component == 3)
            {
                // Convert RGB to RGBA.
                tempBuffer->resize(image.width * image.height * 4);
                for (int y = 0; y < image.height; ++y)
                {
                    const unsigned char *src = image.image.data() + y * image.width * 3;
                    unsigned char *dest = tempBuffer->data() + y * image.width * 4;
                    for (int x = image.width - 1; x >= 0; --x, src += 3, dest += 4)
                    {
                        dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = 255;
                    }
                }

                return tempBuffer->data();
            }
            else if (image.component == 4)
            {
                // Already RGBA, no conversion needed
                return image.image.data();
            }
        }

        return nullptr;
    }
}

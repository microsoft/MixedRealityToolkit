////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include <sstream>
// Implementation is in the Gltf library so this isn't needed: #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "PbrCommon.h"

using namespace Microsoft::WRL;
using namespace DirectX;

#define TRIANGLE_VERTEX_COUNT 3 // #define so it can be used in lambdas without capture

namespace Pbr
{
    namespace Internal
    {
        void ThrowIfFailed(HRESULT hr)
        {
            if (FAILED(hr))
            {
                std::stringstream ss;
                ss << std::hex << "Error in PBR renderer: 0x" << hr;
                throw std::exception(ss.str().c_str());
            }
        }
    }

    const D3D11_INPUT_ELEMENT_DESC Vertex::s_vertexDesc[5] =
    {
        { "POSITION",       0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",         0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",        0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",       0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TRANSFORMINDEX", 0, DXGI_FORMAT_R16_UINT,            0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Based on code from DirectXTK
    PrimitiveBuilder& PrimitiveBuilder::AddSphere(float diameter, uint32_t tessellation, Pbr::NodeIndex_t transformIndex)
    {
        if (tessellation < 3)
        {
            throw std::out_of_range("tesselation parameter out of range");
        }

        const uint32_t verticalSegments = tessellation;
        const uint32_t horizontalSegments = tessellation * 2;

        const float radius = diameter / 2;

        const uint32_t startVertexIndex = (uint32_t)Vertices.size();

        // Create rings of vertices at progressively higher latitudes.
        for (uint32_t i = 0; i <= verticalSegments; i++)
        {
            const float v = 1 - (float)i / verticalSegments;

            const float latitude = (i * XM_PI / verticalSegments) - XM_PIDIV2;
            float dy, dxz;
            XMScalarSinCos(&dy, &dxz, latitude);

            // Create a single ring of vertices at this latitude.
            for (uint32_t j = 0; j <= horizontalSegments; j++)
            {
                const float longitude = j * XM_2PI / horizontalSegments;
                float dx, dz;
                XMScalarSinCos(&dx, &dz, longitude);
                dx *= dxz;
                dz *= dxz;

                // Compute tangent at 90 degrees along longitude.
                float tdx, tdz;
                XMScalarSinCos(&tdx, &tdz, longitude + XM_PI);
                tdx *= dxz;
                tdz *= dxz;

                const XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);
                const XMVECTOR tangent = XMVectorSet(tdx, 0, tdz, 0);

                const float u = (float)j / horizontalSegments;
                const XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

                Pbr::Vertex vert;
                XMStoreFloat3(&vert.Position, normal * radius);
                XMStoreFloat3(&vert.Normal, normal);
                XMStoreFloat4(&vert.Tangent, tangent);
                XMStoreFloat2(&vert.TexCoord0, textureCoordinate);

                vert.ModelTransformIndex = transformIndex;
                Vertices.push_back(vert);
            }
        }

        // Fill the index buffer with triangles joining each pair of latitude rings.
        const uint32_t stride = horizontalSegments + 1;
        const uint32_t startIndicesIndex = (uint32_t)Indices.size();
        for (uint32_t i = 0; i < verticalSegments; i++)
        {
            for (uint32_t j = 0; j <= horizontalSegments; j++)
            {
                uint32_t nextI = i + 1;
                uint32_t nextJ = (j + 1) % stride;

                Indices.push_back(startVertexIndex + (i * stride + j));
                Indices.push_back(startVertexIndex + (nextI * stride + j));
                Indices.push_back(startVertexIndex + (i * stride + nextJ));

                Indices.push_back(startVertexIndex + (i * stride + nextJ));
                Indices.push_back(startVertexIndex + (nextI * stride + j));
                Indices.push_back(startVertexIndex + (nextI * stride + nextJ));
            }
        }

        return *this;
    }

    // Based on code from DirectXTK
    PrimitiveBuilder& PrimitiveBuilder::AddCube(float sideLength, Pbr::NodeIndex_t transformIndex)
    {
        // A box has six faces, each one pointing in a different direction.
        const int FaceCount = 6;

        static const XMVECTORF32 faceNormals[FaceCount] =
        {
            { { {  0,  0,  1, 0 } } },
            { { {  0,  0, -1, 0 } } },
            { { {  1,  0,  0, 0 } } },
            { { { -1,  0,  0, 0 } } },
            { { {  0,  1,  0, 0 } } },
            { { {  0, -1,  0, 0 } } },
        };

        static const XMVECTORF32 textureCoordinates[4] =
        {
            { { { 1, 0, 0, 0 } } },
            { { { 1, 1, 0, 0 } } },
            { { { 0, 1, 0, 0 } } },
            { { { 0, 0, 0, 0 } } },
        };

        // Create each face in turn.
        const float halfSideLength = sideLength / 2;
        for (int i = 0; i < FaceCount; i++)
        {
            XMVECTOR normal = faceNormals[i];
            XMVECTOR tangent = faceNormals[i];

            // Get two vectors perpendicular both to the face normal and to each other.
            XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

            XMVECTOR side1 = XMVector3Cross(normal, basis);
            XMVECTOR side2 = XMVector3Cross(normal, side1);

            // Six indices (two triangles) per face.
            size_t vbase = Vertices.size();
            Indices.push_back((uint32_t)vbase + 0);
            Indices.push_back((uint32_t)vbase + 1);
            Indices.push_back((uint32_t)vbase + 2);

            Indices.push_back((uint32_t)vbase + 0);
            Indices.push_back((uint32_t)vbase + 2);
            Indices.push_back((uint32_t)vbase + 3);

            const XMVECTOR positions[4] =
            {
                { (normal - side1 - side2) * halfSideLength },
                { (normal - side1 + side2) * halfSideLength },
                { (normal + side1 + side2) * halfSideLength },
                { (normal + side1 - side2) * halfSideLength }
            };

            for (int i = 0; i < 4; i++)
            {
                Pbr::Vertex vert;
                XMStoreFloat3(&vert.Position, positions[i]);
                XMStoreFloat3(&vert.Normal, normal);
                XMStoreFloat4(&vert.Tangent, side1); // TODO arbitrarily picked side 1
                XMStoreFloat2(&vert.TexCoord0, textureCoordinates[i]);
                vert.ModelTransformIndex = transformIndex;
                Vertices.push_back(vert);
            }
        }

        return *this;
    }

    namespace Texture
    {
        std::array<uint8_t, 4> XM_CALLCONV CreateRGBA(DirectX::FXMVECTOR color)
        {
            DirectX::XMFLOAT4 colorf;
            DirectX::XMStoreFloat4(&colorf, DirectX::XMVectorScale(color, 255));
            return std::array<uint8_t, 4> { (uint8_t)colorf.x, (uint8_t)colorf.y, (uint8_t)colorf.z, (uint8_t)colorf.w };
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadImage(_In_ ID3D11Device* device, _In_reads_bytes_(size) const uint8_t* fileData, uint32_t fileSize)
        {
            auto freeImageData = [](unsigned char* ptr) { ::free(ptr); };
            using stbi_unique_ptr = std::unique_ptr<unsigned char, decltype(freeImageData)>;

            int w, h, c;
            stbi_unique_ptr rgbaData(stbi_load_from_memory(fileData, fileSize, &w, &h, &c, 4), freeImageData);
            if (!rgbaData)
            {
                throw std::exception("Failed to load image file data.");
            }

            return CreateTexture(device, rgbaData.get(), w * h * c, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateFlatCubeTexture(_In_ ID3D11Device* device, CXMVECTOR color, DXGI_FORMAT format)
        {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = 1;
            desc.Height = 1;
            desc.MipLevels = 1;
            desc.ArraySize = 6;
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

            // Each side is a 1x1 pixel (RGBA) image.
            const std::array<uint8_t, 4> rgbaColor = CreateRGBA(color);
            D3D11_SUBRESOURCE_DATA initData[6];
            for (int i = 0; i < _countof(initData); i++)
            {
                initData[i].pSysMem = rgbaColor.data();
                initData[i].SysMemPitch = initData[i].SysMemSlicePitch = 4;
            }

            ComPtr<ID3D11Texture2D> cubeTexture;
            Internal::ThrowIfFailed(device->CreateTexture2D(&desc, initData, &cubeTexture));

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;

            ComPtr<ID3D11ShaderResourceView> textureView;
            Internal::ThrowIfFailed(device->CreateShaderResourceView(cubeTexture.Get(), &srvDesc, &textureView));

            return textureView;
        }

        ComPtr<ID3D11ShaderResourceView> CreateTexture(_In_ ID3D11Device* device, _In_reads_bytes_(size) const uint8_t* rgba, uint32_t size, int width, int height, DXGI_FORMAT format)
        {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA initData{};
            initData.pSysMem = rgba;
            initData.SysMemPitch = size / height;
            initData.SysMemSlicePitch = size;

            ComPtr<ID3D11Texture2D> texture2D;
            Internal::ThrowIfFailed(device->CreateTexture2D(&desc, &initData, &texture2D));

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

            ComPtr<ID3D11ShaderResourceView> textureView;
            Internal::ThrowIfFailed(device->CreateShaderResourceView(texture2D.Get(), &srvDesc, &textureView));

            return textureView;
        }

        ComPtr<ID3D11SamplerState> CreateSampler(_In_ ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE addressMode)
        {
            CD3D11_SAMPLER_DESC samplerDesc(CD3D11_DEFAULT{});
            samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = addressMode;

            ComPtr<ID3D11SamplerState> samplerState;
            Pbr::Internal::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &samplerState));
            return samplerState;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PaintStrokeSystem.h"
#include <Pbr\PbrMaterial.h>
#include <Pbr\PbrModel.h>
#include <Neso\Engine\EntityStore.h>
#include <Neso\Engine\CommonComponents.h>

using namespace Neso;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;

namespace DemoRoom {

PaintStrokeSystem::PaintStrokeSystem(
    Engine& core,
    std::shared_ptr<Pbr::Resources> pbrResources) :
    System(core),
    m_pbrResources(std::move(pbrResources))
{}

void PaintStrokeSystem::Update(float)
{
    for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<PaintStrokeComponent, PbrRenderable>())
    {
        auto[paintStroke, pbr] = componentSet;

        if (paintStroke->strokeChanged)
        {
            std::shared_ptr<Pbr::Material> strokeMaterial = Pbr::Material::CreateFlat(
                *m_pbrResources,
                *pbr->Color /* base color */,
                0.95f /* roughness */);

            // Load the primitive into D3D buffers with associated materia.
            Pbr::Primitive strokePrimitive(*m_pbrResources, paintStroke->GetPrimitiveData(), std::move(strokeMaterial));

            // Add the primitive into the model.
            if (auto& model = pbr->Model)
            {
                model->Clear();
                model->AddPrimitive(std::move(strokePrimitive));
            }

            paintStroke->strokeChanged = false;
        }
    }
}

void PaintStrokeComponent::AddPoint(const float4x4& transformation, float width)
{
    const float halfWidth = width / 2.0f;

    PaintStrokeComponent::Square square;

    square.TopLeft     = transform(float3{ -halfWidth, 0.0f, +halfWidth }, transformation);
    square.TopRight    = transform(float3{ +halfWidth, 0.0f, +halfWidth }, transformation);
    square.BottomRight = transform(float3{ +halfWidth, 0.0f, -halfWidth }, transformation);
    square.BottomLeft  = transform(float3{ -halfWidth, 0.0f, -halfWidth }, transformation);

    squares.push_back(std::move(square));

    strokeChanged = true;
}

Pbr::PrimitiveBuilder PaintStrokeComponent::GetPrimitiveData()
{
    Pbr::PrimitiveBuilder primitiveBuilder;

    if (squares.size() > 0)
    {
        // Vertices
        std::vector<Pbr::Vertex> vertices;
        vertices.reserve(squares.size() * 4);
        const size_t expectedVerticesCapacity = vertices.capacity();

        constexpr XMFLOAT3 Normal{ 0.0f, 0.0f, 1.0f };
        constexpr XMFLOAT4 Tangent{ 1.0f, 0.0f, 0.0f, 1.0f };
        constexpr XMFLOAT2 TexCoord{ 0.0f, 0.0f };

        for (auto& square : squares)
        {
            vertices.push_back({ AsRef<XMFLOAT3>(square.TopLeft), Normal, Tangent, TexCoord, Pbr::RootNodeIndex });
            vertices.push_back({ AsRef<XMFLOAT3>(square.TopRight), Normal, Tangent, TexCoord, Pbr::RootNodeIndex });
            vertices.push_back({ AsRef<XMFLOAT3>(square.BottomRight), Normal, Tangent, TexCoord, Pbr::RootNodeIndex });
            vertices.push_back({ AsRef<XMFLOAT3>(square.BottomLeft), Normal, Tangent, TexCoord, Pbr::RootNodeIndex });
        }

        constexpr size_t numIndicesPerFace = 6u;
        constexpr size_t numBackFrontFaces = 2u;
        const size_t numSideFaces = 4u * (squares.size() - 1);

        // Indices
        std::vector<uint32_t> indices;
        indices.reserve(numIndicesPerFace * (numBackFrontFaces + numSideFaces));

        const size_t expectedIndicesCapacity = indices.capacity();

        auto addSquare = [&indices](uint32_t topLeft, uint32_t topRight, uint32_t bottomRight, uint32_t bottomLeft)
        {
            indices.push_back(topLeft); indices.push_back(topRight); indices.push_back(bottomRight);
            indices.push_back(topLeft); indices.push_back(bottomRight); indices.push_back(bottomLeft);
        };

        // Back face
        addSquare(1, 0, 3, 2);

        // Front face
        const uint32_t frontStart = static_cast<uint32_t>(4 * (squares.size() - 1));
        addSquare(frontStart, frontStart + 1, frontStart + 2, frontStart + 3);

        // Side faces
        const uint32_t sidesCount = static_cast<uint32_t>(squares.size() - 1);
        for (uint32_t sideIndex = 0; sideIndex < sidesCount; sideIndex++)
        {
            const uint32_t start = sideIndex * 4u;

            // +0 = back / top / left
            // +1 = back / top / right
            // +2 = back / bottom / right
            // +3 = back / bottom / left
            // +4 = front / top / left
            // +5 = front / top / right
            // +6 = front / bottom / right
            // +7 = front / bottom / left

            addSquare(start + 0, start + 1, start + 5, start + 4); // Top
            addSquare(start + 5, start + 1, start + 2, start + 6); // Right
            addSquare(start + 0, start + 4, start + 7, start + 3); // Left
            addSquare(start + 7, start + 6, start + 2, start + 3); // Bottom
        }

        // Make sure we only allocate memory once
        fail_fast_if(expectedVerticesCapacity != vertices.size());
        fail_fast_if(expectedIndicesCapacity != indices.size());

        primitiveBuilder.Vertices = std::move(vertices);
        primitiveBuilder.Indices = std::move(indices);
    }

    return primitiveBuilder;
}

} // namespace DemoRoom

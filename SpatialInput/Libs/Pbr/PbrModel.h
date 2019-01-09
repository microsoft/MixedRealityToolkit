////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <wrl\client.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <DirectXMath.h>
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrPrimitive.h"

namespace Pbr
{
    // Node for creating a hierarchy of transforms. These transforms are referenced by vertices in the model's primitives.
    struct Node
    {
        using Collection = std::vector<Node>;

        Node(DirectX::CXMMATRIX localTransform, std::string name, NodeIndex_t index, NodeIndex_t parentNodeIndex)
            : Name(std::move(name)), Index(index), ParentNodeIndex(parentNodeIndex)
        {
            SetTransform(localTransform);
        }

        // Set the local transform for this node.
        void XM_CALLCONV SetTransform(DirectX::FXMMATRIX transform)
        {
            DirectX::XMStoreFloat4x4(&m_localTransform, transform);
            InterlockedIncrement(&m_modifyCount);
        }

        // Get the local transform for this node.
        DirectX::XMMATRIX XM_CALLCONV GetTransform() const
        {
            return DirectX::XMLoadFloat4x4(&m_localTransform);
        }

        const std::string Name;
        const NodeIndex_t Index;
        const NodeIndex_t ParentNodeIndex;

    private:
        friend struct Model;
        uint32_t m_modifyCount{ 0 };
        DirectX::XMFLOAT4X4 m_localTransform;
    };

    // A model is a collection of primitives (which reference a material) and transforms referenced by the primitives' vertices.
    struct Model
    {
        Model(bool createRootNode = true);

        // Add a node to the model.
        Node& XM_CALLCONV AddNode(DirectX::FXMMATRIX transform, NodeIndex_t parentIndex, std::string name);

        // Add a primitive to the model.
        void AddPrimitive(Primitive primitive);

        // Render the model.
        void Render(Pbr::Resources const& pbrResources, _In_ ID3D11DeviceContext3* context) const;

        // Remove all primitives.
        void Clear();

        // Create a clone of this model.
        std::shared_ptr<Model> Clone(Pbr::Resources const& pbrResources) const;

        NodeIndex_t GetNodeCount() const { return (NodeIndex_t)m_nodes.size(); }
        Node& GetNode(NodeIndex_t nodeIndex) { return m_nodes[nodeIndex]; }
        const Node& GetNode(NodeIndex_t nodeIndex) const { return m_nodes[nodeIndex]; }

        // Find the first node which matches a given name.
        std::optional<NodeIndex_t> FindFirstNode(char const* name, std::optional<NodeIndex_t> const& parentNodeIndex = {}) const;

        // Compute the world transform for a given node.
        DirectX::XMMATRIX GetNodeWorldTransform(NodeIndex_t nodeIndex) const;

        uint32_t GetPrimitiveCount() const { return (uint32_t)m_primitives.size(); }
        Primitive& GetPrimitive(uint32_t index) { return m_primitives[index]; }
        const Primitive& GetPrimitive(uint32_t index) const { return m_primitives[index]; }

        std::string Name;

    private:
        // Updated the transforms used to render the model. This needs to be called any time a node transform is changed.
        void UpdateTransforms(Pbr::Resources const& pbrResources, _In_ ID3D11DeviceContext3* context) const;

    private:
        // A model is made up of one or more Primitives. Each Primitive has a unique material.
        // Ideally primitives with the same material should be merged to reduce draw calls.
        Primitive::Collection m_primitives;

        // A model contains one or more nodes. Each vertex of a primitive references a node to have the
        // node's transform applied.
        Node::Collection m_nodes;

        // Temporary buffer holds the world transforms, computed from the node's local transforms.
        mutable std::vector<DirectX::XMFLOAT4X4> m_modelTransforms;
        mutable Microsoft::WRL::ComPtr<ID3D11Buffer> m_modelTransformsStructuredBuffer;
        mutable Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_modelTransformsResourceView;

        mutable uint32_t TotalModifyCount{ 0 };
    };
}

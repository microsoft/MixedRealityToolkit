#include "pch.h"
#include "HalfEdgeMesh.h"

using namespace DirectX;

namespace
{
    HalfEdgeMesh::Edge* FindExistingEdge(HalfEdgeMesh::Edge* edge, int32_t endIndex)
    {
        while (edge != nullptr)
        {
            if (edge->next->vertex == endIndex)
            {
                return edge;
            }
            edge = edge->nextVertexEdge;
        }
        return nullptr;
    }
}

HalfEdgeMesh::~HalfEdgeMesh()
{
    for (HalfEdgeMesh::Edge* edge : m_spNewEdges)
    {
        delete edge;
    }
}

bool HalfEdgeMesh::AddTriangle(_In_ const TriangleINT32& triangle, _Out_ HalfEdgeMesh::Edge **edge)
{
    Edge* edge1 = new Edge{0};
    Edge* edge2 = new Edge{0};
    Edge* edge3 = new Edge{0};

    m_spNewEdges.push_back(edge1);
    m_spNewEdges.push_back(edge2);
    m_spNewEdges.push_back(edge3);

    bool ret = AddTriangle(triangle, edge1, edge2, edge3);
    

    *edge = edge1;
    return ret;
}

bool HalfEdgeMesh::AddTriangle(
    _In_ const TriangleINT32& triangle, 
    _In_ HalfEdgeMesh::Edge* edge1,
    _In_ HalfEdgeMesh::Edge* edge2,
    _In_ HalfEdgeMesh::Edge* edge3)
{
    const int32_t vertex1 = triangle.at(0);
    const int32_t vertex2 = triangle.at(1);
    const int32_t vertex3 = triangle.at(2);
    bool nonManifold = false;

    ASSERT(vertex1 != vertex2);
    ASSERT(vertex2 != vertex3);
    ASSERT(vertex3 != vertex1);

    edge1->vertex = vertex1;
    Edge *pair = FindExistingEdge(m_spVertices[vertex2], vertex1);
    if (pair != nullptr && pair->pair == nullptr) 
    {
        edge1->pair = pair;
        pair->pair = edge1; 
    }
    else if (pair != nullptr)
    {
        // we are generating non-manifold mesh - return an error
        nonManifold = true;
    }

    edge2->vertex = vertex2;
    pair = FindExistingEdge(m_spVertices[vertex3], vertex2);
    if (pair != nullptr && pair->pair == nullptr)
    {
        edge2->pair = pair;
        pair->pair = edge2;
    }
    else if (pair != nullptr)
    {
        // we are generating non-manifold mesh - return an error
        nonManifold = true;
    }

    edge3->vertex = vertex3;
    pair = FindExistingEdge(m_spVertices[vertex1], vertex3);
    if (pair != nullptr && pair->pair == nullptr)
    {
        edge3->pair = pair;
        pair->pair = edge3;
    }
    else if (pair != nullptr)
    {
        // we are generating non-manifold mesh - return an error
        nonManifold = true;
    }

    if (m_spVertices[vertex1])
    {
        edge1->nextVertexEdge = m_spVertices[vertex1]->nextVertexEdge;
        m_spVertices[vertex1]->nextVertexEdge = edge1;
    }
    else
    {
        m_spVertices[vertex1] = edge1;
    }

    if (m_spVertices[vertex2])
    {
        edge2->nextVertexEdge = m_spVertices[vertex2]->nextVertexEdge;
        m_spVertices[vertex2]->nextVertexEdge = edge2;
    }
    else
    {
        m_spVertices[vertex2] = edge2;
    }

    if (m_spVertices[vertex3])
    {
        edge3->nextVertexEdge = m_spVertices[vertex3]->nextVertexEdge;
        m_spVertices[vertex3]->nextVertexEdge = edge3;
    }
    else
    {
        m_spVertices[vertex3] = edge3;
    }

    edge1->next = edge2;
    edge2->next = edge3;
    edge3->next = edge1;

    return !nonManifold;
}

// remove edges from the mesh in the range of [startOffset, endOffset).
void HalfEdgeMesh::RemoveNewEdges(_In_ const uint32_t startOffset, _In_ const uint32_t endOffset)
{
    for (uint32_t i = startOffset; i < endOffset; ++i)
    {
        Edge* edge = m_spNewEdges[i];
        if (edge->pair)
        {
            edge->pair->pair = nullptr;
        }

        Edge* vertexNeighbor = m_spVertices[edge->vertex];
        if (vertexNeighbor == edge)
        {
            m_spVertices[edge->vertex] = edge->nextVertexEdge;
        }
        else
        {
            while (vertexNeighbor->nextVertexEdge && vertexNeighbor->nextVertexEdge != edge)
            {
                vertexNeighbor = vertexNeighbor->nextVertexEdge;
            }
            vertexNeighbor->nextVertexEdge = edge->nextVertexEdge;
        }

        delete edge;
    }

    m_spNewEdges.erase(m_spNewEdges.begin() + startOffset, m_spNewEdges.begin() + endOffset);
}

HalfEdgeMesh::TriangleINT32 HalfEdgeMesh::GetTriangle(_In_ const HalfEdgeMesh::Edge* edge)
{
    const HalfEdgeMesh::Edge* next = edge->next;
    const TriangleINT32 triangle = {
        edge->vertex,
        next->vertex,
        next->next->vertex
    };
    return move(triangle);
}
#pragma once
#include <array>
#include <vector>
#include <stdint.h>
#include <memory>
#include "MathHelpers.h"

/*! 
    Half Edge meshes represent a mesh by using half edges.  There are three of these half edges per triangle.  They are called half because they are 
    half of a Winged edge representation.  They are an edge with a direction that goes around a single triangle, three to a loop.  Adjacent triangles will 
    have an edge going the oposite direction.  Together these two half edges make a whole and represent all the data in a Winged edge.  The big 
    advantage of Half Egde vs Winged Edge representation is that with half edge you don't have to conditionally check which direction you are going.
*/

class HalfEdgeMesh
{
public:
    typedef array<int32_t, VERTICES_PER_TRIANGLE> TriangleINT32;

    struct Edge
    {
        int32_t vertex;       // starting vertex for this half edge.
        Edge* next;           // next half-edge of the face containing this half-edge
        Edge* nextVertexEdge; // pointer to the next edge that shares the same starting vertex.
        Edge* pair;           // shared edge in the opposite direction (from next->vertex to vertex).
    };

    // indexData is an array of indices to a triangle list.  Three per triangle.
    HalfEdgeMesh(_In_ uint32_t numVertices, _In_ uint32_t numIndices, _In_ const TriangleINT32* indexData)
    {
        ASSERT(numIndices % VERTICES_PER_TRIANGLE == 0);

        m_spEdges.resize(numIndices, { 0 });
        m_spVertices.resize(numVertices, { 0 });

        // Create half-edges.
        for (uint32_t faceIndex = 0; faceIndex < numIndices / VERTICES_PER_TRIANGLE; faceIndex++)
        {
            const TriangleINT32 triangle = {
                indexData[faceIndex].at(0),
                indexData[faceIndex].at(1),
                indexData[faceIndex].at(2)
            };

            Edge* edge1 = &m_spEdges[faceIndex * VERTICES_PER_TRIANGLE];
            Edge* edge2 = &m_spEdges[faceIndex * VERTICES_PER_TRIANGLE + 1];
            Edge* edge3 = &m_spEdges[faceIndex * VERTICES_PER_TRIANGLE + 2];

            VERIFY(AddTriangle(triangle, edge1, edge2, edge3)); // we should assume that the intial mesh is manifold (two vertices will be shared between at most 2 half-edges)
        }
    }

    ~HalfEdgeMesh();

    bool AddTriangle(_In_ const TriangleINT32& triangle, _Out_ Edge** triangleEdge);
    void RemoveNewEdges(_In_ const uint32_t startOffset, _In_ const uint32_t endOffset);

    static TriangleINT32 GetTriangle(_In_ const HalfEdgeMesh::Edge* edge);

    vector<Edge>  m_spEdges;
    vector<Edge*> m_spVertices;
    // TODO: replace with memory pool.  This is a temporary fix to allow for adding additional edges.
    vector<Edge*> m_spNewEdges;

    class VertexNeighborSet
    {
    private: 
        class VertexNeighbor
        {
        public:
            uint32_t operator*()
            {
                return m_edge->next->vertex;
            }

            VertexNeighbor &operator++()
            {
                m_edge = m_edge->nextVertexEdge;
                return *this;
            }

            bool operator!=(const VertexNeighbor& other)
            {
                return m_edge != other.m_edge;
            }


            VertexNeighbor(Edge* edge) : m_edge(edge) {}

        private:
            Edge* m_edge;
        };

    public:
        VertexNeighbor begin() { return VertexNeighbor(m_vert); }
        VertexNeighbor end() { return VertexNeighbor(nullptr); }

        VertexNeighborSet(Edge* first) : m_vert(first)
        {}

    private:
        Edge *m_vert;
    };

    VertexNeighborSet GetNeighborVerts(uint32_t vert)
    {
        return VertexNeighborSet(m_spVertices[vert]);
    }

    // similar iterator for walking vertices connected to the given vertex by incoming our outgoing edges
    // incoming edges are included if they have a null pair (there is no corresponding outgoing vertex)

    struct DigraphEdge
    {
        INT32 vert; // neighbor vertex
        Edge* edge; // outgoing edge for the triangle we are considering
        bool flipped;
    };

    class DigraphVertexNeighborSet
    {
    private:
        class VertexNeighbor
        {
        private:
            DigraphEdge m_edge;

        public:
            DigraphEdge operator*()
            {
                // if we are looking at the incoming edge for this face, need to look at next->next edge's start vertex
                return m_edge;
            }

            VertexNeighbor &operator++()
            {
                if (m_edge.flipped || m_edge.edge->next->next->pair != nullptr)
                {
                    // move to the next outgoing edge
                    m_edge.flipped = false;
                    m_edge.edge = m_edge.edge->nextVertexEdge;
                }
                else
                {
                    // consider the incoming edge that is part of this face
                    m_edge.flipped = true;
                }

                // the end vertex:
                if (m_edge.edge != nullptr)
                {
                    m_edge.vert = m_edge.flipped ? m_edge.edge->next->next->vertex : m_edge.edge->next->vertex;
                }
                else
                {
                    m_edge.flipped = false;
                    m_edge.vert = 0;
                }

                return *this;
            }

            bool operator!=(const VertexNeighbor& other)
            {
                return (m_edge.edge != other.m_edge.edge) || (m_edge.flipped != other.m_edge.flipped);
            }

            VertexNeighbor(Edge* edge) : m_edge({ edge->next->vertex, edge, false }) {}

        };

    public:
        VertexNeighbor begin() { return VertexNeighbor(m_vert); }
        VertexNeighbor end() { return VertexNeighbor(nullptr); }

        DigraphVertexNeighborSet(Edge* first) : m_vert(first)
        {}

    private:
        Edge *m_vert;
    };

    DigraphVertexNeighborSet GetDigraphNeighborVerts(uint32_t vert)
    {
        return DigraphVertexNeighborSet(m_spVertices[vert]);
    }

    bool IsCoallesced(uint32_t vert)
    {
        // determines if this is a vertex that should be ignored by algorithms because it was coallesced to another vertex
        return m_spVertices[vert] != nullptr && m_spVertices[vert]->vertex != vert;
    }

private:
    bool AddTriangle(
        _In_ const TriangleINT32& triangle,
        _In_ HalfEdgeMesh::Edge* edge1,
        _In_ HalfEdgeMesh::Edge* edge2,
        _In_ HalfEdgeMesh::Edge* edge3);
};


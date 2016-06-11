#include "pch.h"
#include "PlaneFinding.h"
#include "HalfEdgeMesh.h"
#include "PCAHelper.h"
#include "NBest.h"
#include "Util.h"

using namespace DirectX;

namespace PlaneFinding
{
    // How we are finding planes:
    //  * First we calculate the curvature for every vertex
    //  * Next, we flood-fill planar regions, and select the best regions as potential planes
    //  * We then generate plane equations for these regions using Principal Component Analysis (PCAHelper)
    //  * Once we have plane equations, re-floodfill to generate our final set of vertices for each plane
    //  * The output of this is a collection of planes, each is a connected set of vertices.

    // TODO: allow these constants to be modified at runtime, or configured by regkey to determine ideal values
    // constants for first pass generating potential planar regions
    const float cLowCurvatureThreshold = 0.1f; // vertices where the curvature is sufficiently low are consider potentially planar
    const float cMaxDotForNeighbors = 0.89f; // vertices are considered potentially coplanar if their normals differ by some maximum angle (measured as dot product)

                                             // constants for filtering how many potential planes are considered
    const UINT32 cMaxPlanesPerSurface = 30; // constraints on how many planes we can find in a single volume
    const UINT32 cMinVertsPerPlane = 10; // minimum vertices required for a planar region to count as a plane
    const float cMinimumPlaneSize = 0.125f; // threshold for how large a plane must be when projected onto the tangent/cotangent vectors.  Measured as standard deviation of vertices, in meters.

                                            // constants used for bucketing vertices to planes
    const float cMaxDistanceFromPlane = 0.125f; // max distance from a plane equation for a vetex to be considered part of the plane, in meters
    const float cMinCosAngleBetweenNormalAndPlane = 0.5f * sqrtf(3.0f); // min angle between plane and vertex normal to consider vertex part of the plane (30 degrees)

    const UINT32 INVALID_PLANE = static_cast<UINT32>(-1); // used to identify invalid planes

    const XMVECTOR cUpDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // PlaneData represents information about a set of points we consider to be coplanar
    // it contains data that can be used to determine the plane equation
    class PlaneData
    {
    private:
        // data used to identify the plane
        UINT32 m_planeId = INVALID_PLANE;
        UINT32 m_startVertex; // used for a start for flood filling (could be multiple vertices when we span blocks/volumes)
        XMFLOAT3 m_normal; // used for flood-filling

                           // data used for sorting how big this plane is
        UINT32 m_numVerts = 0;
        XMFLOAT3 m_sum = { 0.0f, 0.0f, 0.0f };

        // data used to store the plane equation
        Plane m_equation;
        XMFLOAT3 m_tangent;

        // bounds of the plane in plane space (y = tangent, z = -normal, x satisfies left-handed coordinate system given y/z)
        XMFLOAT3 m_min = { FLT_MAX, FLT_MAX, FLT_MAX };
        XMFLOAT3 m_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        UINT32 m_extremeVertIndex = 0;

        bool m_isGravityAligned = false;

        // indicates whether we want to actually use this plane, or throw it away
        bool m_ignore = false;

    public:
        bool operator<(_In_ PlaneData &p)
        {
            return m_numVerts < p.m_numVerts;
        }

        PlaneData(UINT32 id, UINT32 startIndex, XMFLOAT3 normal) : m_normal(normal), m_startVertex(startIndex), m_planeId(id)
        {}

        PlaneData()
        {}

        void ResetCountAndSum()
        {
            // we could make multiple flood-fill passes
            m_sum = { 0.0f, 0.0f, 0.0f };
            m_numVerts = 0;
        }

        UINT32 GetPlaneId()
        {
            return m_planeId;
        }

        XMFLOAT3 GetMean()
        {
            if (m_numVerts == 0) {
                return { 0.0f, 0.0f, 0.0f };
            }
            return { m_sum.x / m_numVerts, m_sum.y / m_numVerts, m_sum.z / m_numVerts };
        }

        void AddVertex(XMFLOAT3 vertex)
        {
            XMVECTOR v = XMLoadFloat3(&vertex);
            XMFLOAT3 vert;
            XMStoreFloat3(&vert, v);

            m_sum.x += vert.x;
            m_sum.y += vert.y;
            m_sum.z += vert.z;
            m_numVerts++;
        }

        void AddVertexAndUpdateBounds(XMFLOAT3 vertex, UINT32 vertIndex)
        {
            AddVertex(vertex);

            XMVECTOR vertInMeshSpace = XMLoadFloat3(&vertex);
            XMFLOAT3 vertInPlaneSpace;

            XMStoreFloat3(&vertInPlaneSpace, XMVector3TransformCoord(vertInMeshSpace, GetMeshToPlaneRotation()));
            m_min.x = min(m_min.x, vertInPlaneSpace.x);
            m_min.y = min(m_min.y, vertInPlaneSpace.y);
            m_min.z = min(m_min.z, vertInPlaneSpace.z);
            m_max.x = max(m_max.x, vertInPlaneSpace.x);
            m_max.y = max(m_max.y, vertInPlaneSpace.y);
            m_max.z = max(m_max.z, vertInPlaneSpace.z);
            if (m_max.x == vertInPlaneSpace.x)
            {
                m_extremeVertIndex = vertIndex;
            }
        }

        void SetPlaneEquationData(Plane equation, XMFLOAT3 tangent)
        {
            m_equation = equation;
            m_tangent = tangent;

            // normal should be in the same direction as m_normal, negate if necessary
            auto normal = m_equation.normal;
            float dot = normal.x*m_normal.x + normal.y * m_normal.y + normal.z * m_normal.z;
            if (dot < 0)
            {
                m_equation.normal.x *= -1;
                m_equation.normal.y *= -1;
                m_equation.normal.z *= -1;
                m_equation.d *= -1;
            }
        }

        void GetPlaneEquationData(_Out_ Plane *equation, _Out_ XMFLOAT3 *tangent)
        {
            *equation = m_equation;
            *tangent = m_tangent;
        }

        void SetIsGravityAligned(bool isGravityAligned)
        {
            m_isGravityAligned = isGravityAligned;
        }

        bool IsGravityAligned()
        {
            return m_isGravityAligned;
        }

        void IgnorePlane()
        {
            m_ignore = true;
        }

        bool ShouldIgnorePlane()
        {
            return m_ignore;
        }

        UINT32 GetNumVertices()
        {
            return m_numVerts;
        }

        UINT32 GetStartVertexIndex()
        {
            return m_startVertex;
        }

        XMMATRIX GetPlaneToMeshRotation()
        {
            // plane space is z=-normal, y=dominant tangent, x=orthogonal

            XMMATRIX rotationMatrix = XMMatrixIdentity();
            rotationMatrix.r[2] = -XMLoadFloat3(&m_equation.normal); // negative normal, so if you are looking at the plane, the normal is pointing backwards
            rotationMatrix.r[1] = XMLoadFloat3(&m_tangent);
            rotationMatrix.r[0] = XMVector3Cross(rotationMatrix.r[1], rotationMatrix.r[2]);

            // cross product should behave the left-hand rule for our coordinate system:
            // z cross y = x

            return rotationMatrix;
        }

        XMMATRIX GetMeshToPlaneRotation()
        {
            return XMMatrixTranspose(GetPlaneToMeshRotation());
        }

        BoundingOrientedBox GetBoundsInMeshSpace(_In_ XMFLOAT3 *verts, UINT32 cVerts, _In_ UINT32 *vertColors)
        {
            auto includeVert = [&](UINT32 index)
            {
                bool ret = false;
                if (vertColors[index] == m_planeId)
                {
                    // we could do more filtering - only include vertices on holes, with neighbors that aren't in the plane, or are not contained within their neighbors when projected to the plane
                    // TODO: consider this as a potential perf optimization
                    ret = true;

                }
                return ret;
            };

            UINT32 index = 0;
            const XMMATRIX meshToPlaneTransform = GetMeshToPlaneRotation();

            // If the plane is gravity aligned, then simply fit an axis bounding aligned box in the plane space
            // and don't try to optimize to the tightest fitting oriented bounding box.
            bool findTightestBounds = !IsGravityAligned();
            auto bestBoxInPlaneSpace = GetBoundsInOrientedSpace(findTightestBounds, [&](XMFLOAT3 *vertInOrientedSpace) -> bool
            {
                while (index < cVerts)
                {
                    if (includeVert(index))
                    {
                        XMStoreFloat3(vertInOrientedSpace, XMVector3TransformCoord(XMLoadFloat3(verts + index), meshToPlaneTransform));
                        index++;
                        return true;
                    }
                    index++;
                }
                return false;
            });

            BoundingOrientedBox xmBoundsInMeshSpace;
            bestBoxInPlaneSpace.Transform(xmBoundsInMeshSpace, XMMatrixTranspose(meshToPlaneTransform));
            return xmBoundsInMeshSpace;
        }
    };

    struct PerVertexData
    {
        float Curvature; // how flat is this vertex (how far is this verticies normal from its neighbors)
        UINT32 plane;
    };

    float Dot(const XMFLOAT3& n1, const XMFLOAT3& n2)
    {
        return (n1.x * n2.x) + (n1.y * n2.y) + (n1.z * n2.z);
    }

    void FillVertexCurvatures(_Out_ vector<PerVertexData> *vertexData, _In_ HalfEdgeMesh *halfEdge, _In_ XMFLOAT3 *normals, _In_ UINT32 vertCount)
    {
        for (UINT32 i = 0; i < vertCount; ++i)
        {
            float dSum = 0.0f;
            const XMFLOAT3 normal = normals[i];

            // find the curvature
            int numNeighbors = 0;
            for (UINT32 neighbor : halfEdge->GetNeighborVerts(i))
            {
                dSum += Dot(normal, normals[neighbor]);
                numNeighbors++;
            }

            if (numNeighbors == 0)
            {
                ASSERT(false); // we expect no floating vertices without neighbors
                numNeighbors = 1;
            }

            // the curvature
            vertexData->push_back({ 1.0f - dSum / (numNeighbors), INVALID_PLANE });
        }
    }

    void SmoothCurvatures(_Inout_ vector<PerVertexData> *vertexData, _In_ HalfEdgeMesh *halfEdge, _In_ UINT32 vertCount)
    {
        // smooth the curvature
        // TODO: should be done on a second array to avoid consuming smoothed data, but
        // avoiding that now to avoid creating yet another large buffer
        for (UINT32 i = 0; i < vertCount; ++i)
        {
            // find the normal of each neighbor
            int numNeighbors = 1;
            float dSum = (*vertexData)[i].Curvature;
            for (UINT32 neighbor : halfEdge->GetNeighborVerts(i))
            {
                dSum += (*vertexData)[neighbor].Curvature;
                numNeighbors++;
            }
            (*vertexData)[i].Curvature = dSum / numNeighbors;
        }
    }

    template < typename TFunc >
    void FloodFillVertices(_In_ HalfEdgeMesh *halfEdge, UINT32 startVert, _In_ const TFunc &func)
    {
        if (halfEdge->m_spVertices[startVert] != nullptr)
        {
            // we flood-fill from coallesced vertex
            startVert = halfEdge->m_spVertices[startVert]->vertex;
        }

        queue<UINT32> toExpand;
        toExpand.push(startVert);

        while (!toExpand.empty())
        {
            UINT32 vert = toExpand.front();
            toExpand.pop();

            for (UINT32 neighbor : halfEdge->GetNeighborVerts(vert))
            {
                if (func(neighbor))
                {
                    toExpand.push(neighbor);
                }
            }
        }
    }

    void FloodFillLowCurvatureRegions(_Inout_ vector<PerVertexData> *pVertexData, _In_ HalfEdgeMesh *halfEdge, _In_ XMFLOAT3 *normals, _In_ XMFLOAT3 *verts, UINT32 vertCount, _Out_ NBest<cMaxPlanesPerSurface, PlaneData> *bestPlanes)
    {
        vector<PerVertexData> &vertexData = *pVertexData;

        UINT32 nextPlane = 1; // assign an id to planar regions
        for (UINT32 i = 0; i < vertCount; ++i)
        {
            if (!halfEdge->IsCoallesced(i)) // don't process coallesced vertices
            {
                if (vertexData[i].plane == INVALID_PLANE && vertexData[i].Curvature < cLowCurvatureThreshold)
                {
                    // this vertex is not already labelled - start a new region
                    vertexData[i].plane = nextPlane;
                    const XMFLOAT3 normal = normals[i];
                    const XMFLOAT3 vertex = verts[i];
                    PlaneData planeData = PlaneData(nextPlane, i, normal);

                    // flood fill neighbors with low enough curvature
                    FloodFillVertices(halfEdge, i, [&](UINT32 vert)
                    {
                        bool ret = ((vertexData[vert].plane == INVALID_PLANE) &&
                            (vertexData[vert].Curvature < cLowCurvatureThreshold) &&
                            (Dot(normals[vert], normal) > cMaxDotForNeighbors));

                        if (ret)
                        {
                            vertexData[vert].plane = nextPlane;
                            planeData.AddVertex(verts[vert]);
                        }

                        return ret; // add continue filling this node
                    });

                    // we could get a plane with low number of verts - we require at least 3
                    if (planeData.GetNumVertices() > cMinVertsPerPlane)
                    {
                        bestPlanes->Add(planeData);
                    }

                    nextPlane++;
                }
            }
        }
    }

    void GeneratePlaneEquations(_Inout_ vector<PerVertexData> *pVertexData, _In_ HalfEdgeMesh *halfEdge, UINT32 vertCount, _In_ XMFLOAT3 *verts, _Inout_ NBest<cMaxPlanesPerSurface, PlaneData> *bestPlanes, _In_ const float MeshToMetersScale, _In_ float snapToGravityThreshold, _In_ const XMVECTOR& vUpInSurfaceSpace)
    {
        map<UINT32, PCAHelper> pcaMap;

        // generate the plane equation for each plane
        for (unsigned int i = 0; i < bestPlanes->num; ++i)
        {
            PCAHelper pca;
            pca.SetMean(bestPlanes->best[i].GetMean());
            pcaMap[bestPlanes->best[i].GetPlaneId()] = pca;
        }

        for (unsigned int i = 0; i < vertCount; ++i)
        {
            if (!halfEdge->IsCoallesced(i)) // don't process coallesced vertices
            {
                auto pca = pcaMap.find((*pVertexData)[i].plane);
                if (pca != pcaMap.end())
                {
                    XMFLOAT3 vert;
                    XMStoreFloat3(&vert, XMLoadFloat3(&verts[i]));
                    pca->second.AddVertex(vert);
                }
            }
        }

        for (unsigned int i = 0; i < bestPlanes->num; ++i)
        {
            auto pca = pcaMap[bestPlanes->best[i].GetPlaneId()];
            pca.Solve();
            XMFLOAT3 stdDevs = pca.GetStandardDeviations();
            if (stdDevs.x < cMinimumPlaneSize / MeshToMetersScale || stdDevs.y < cMinimumPlaneSize / MeshToMetersScale)
            {
                // throw this plane away - it is not large enough in one of the tangent directions
                bestPlanes->best[i].IgnorePlane();
            }
            else
            {
                auto plane = pca.GetPlaneEquation();
                auto tangent = pca.GetTangent();

                if (snapToGravityThreshold != 0.0f)
                {
                    bool isGravityAligned = SnapToGravity(&plane, &tangent, bestPlanes->best[i].GetMean(), snapToGravityThreshold, vUpInSurfaceSpace);
                    bestPlanes->best[i].SetIsGravityAligned(isGravityAligned);
                }

                bestPlanes->best[i].SetPlaneEquationData(plane, tangent);
            }
        }
    }

    void FloodFillPlaneEquation(_Inout_ vector<PerVertexData> *pVertexData, UINT32 vertCount, _In_ HalfEdgeMesh *halfEdge, _In_ XMFLOAT3 *normals, _In_ XMFLOAT3 *verts, _Inout_ NBest<cMaxPlanesPerSurface, PlaneData> *bestPlanes, const float meshToMetersScale)
    {
        for (UINT32 i = 0; i < vertCount; ++i)
        {
            (*pVertexData)[i].plane = INVALID_PLANE;
        }

        const float cMaxDistanceFromPlaneInMeshSpace = cMaxDistanceFromPlane / meshToMetersScale;

        for (UINT32 i = 0; i < bestPlanes->num; ++i)
        {
            if (!bestPlanes->best[i].ShouldIgnorePlane())
            {
                bestPlanes->best[i].ResetCountAndSum();
                UINT32 startVert = bestPlanes->best[i].GetStartVertexIndex();

                Plane planeEq;
                XMFLOAT3 tangent;
                bestPlanes->best[i].GetPlaneEquationData(&planeEq, &tangent);
                XMVECTOR plane = planeEq.AsVector();

                UINT32 planeId = bestPlanes->best[i].GetPlaneId();

                FloodFillVertices(halfEdge, startVert, [&](UINT32 vertIndex)
                {
                    // return true if
                    // 1. this vertex isn't already considered part of a plane (on the second pass)
                    // 2.  the vertex and normal are in the appropriate range to consider

                    XMVECTOR vert = XMLoadFloat3(&verts[vertIndex]);
                    float distance = XMVectorGetX(XMVectorAbs(XMPlaneDotCoord(plane, vert)));

                    XMVECTOR normal = XMLoadFloat3(&normals[vertIndex]); // normalized
                    float cosAngle = XMVectorGetX(XMPlaneDotNormal(plane, normal)); // normalized

                    bool expand = (((*pVertexData)[vertIndex].plane == INVALID_PLANE) &&
                        (distance < cMaxDistanceFromPlaneInMeshSpace) &&
                        (cosAngle > cMinCosAngleBetweenNormalAndPlane));

                    // if this vertex is close enough to our plane equation
                    if (expand)
                    {
                        bestPlanes->best[i].AddVertexAndUpdateBounds(verts[vertIndex], vertIndex);
                        (*pVertexData)[vertIndex].plane = planeId;
                    }
                    return expand;
                });

                if (bestPlanes->best[i].GetNumVertices() < 4)
                {
                    bestPlanes->best[i].IgnorePlane();

                    // Reset the planeId for the verts that were part of this plane
                    for (UINT32 vertIndex = 0; vertIndex < vertCount; ++vertIndex)
                    {
                        if ((*pVertexData)[vertIndex].plane == planeId)
                        {
                            (*pVertexData)[vertIndex].plane = INVALID_PLANE;
                        }
                    }
                }
            }
        }
    }

    float GetArea(_In_ UINT32 plane, _In_ vector<UINT32> &vertexPlaneMapping, _In_ UINT32 numIndices, _In_ XMFLOAT3 *verts, _In_ INT32 *indices)
    {
        float calcArea = 0.0;
        for (UINT32 i = 0; i < numIndices; i += 3)
        {
            if (vertexPlaneMapping[indices[i]] == plane &&
                vertexPlaneMapping[indices[i]] == vertexPlaneMapping[indices[i + 1]] &&
                vertexPlaneMapping[indices[i]] == vertexPlaneMapping[indices[i + 2]])
            {
                // all vertices are in the same plane - not part of the remainder
                XMVECTOR v1 = XMLoadFloat3(verts + indices[i]);
                XMVECTOR v2 = XMLoadFloat3(verts + indices[i + 1]);
                XMVECTOR v3 = XMLoadFloat3(verts + indices[i + 2]);
                calcArea += XMVectorGetX(XMVector3Length(XMVector3Cross(v3 - v2, v3 - v1))) / 2.0f;
            }
        }

        return calcArea;
    }

    vector<BoundedPlane> FindPlanes(
        _In_ INT32 numMeshes,
        _In_count_(numMeshes) MeshData* meshes,
        _In_ float snapToGravityThreshold)
    {
        vector<BoundedPlane> planes;

        for (int i = 0; i < numMeshes; ++i)
        {
            UINT32 vertCount = meshes[i].vertCount;
            UINT32 numIndices = meshes[i].indexCount;
            XMFLOAT3 *verts = meshes[i].verts;
            XMFLOAT3 *normals = meshes[i].normals;
            INT32* indices = meshes[i].indices;
            XMFLOAT4X4 transform = meshes[i].transform;

            vector<PerVertexData> vertexData;
            vertexData.reserve(vertCount);

            HalfEdgeMesh halfEdge = HalfEdgeMesh(vertCount, numIndices, reinterpret_cast<array<INT32, VERTICES_PER_TRIANGLE>*>(indices));

            XMMATRIX surfaceToObserver = XMLoadFloat4x4(&transform);
            float meshToMetersScale = XMVectorGetX(XMVector3Length(surfaceToObserver.r[0]));

            // First we calculate the curvature for every vertex
            FillVertexCurvatures(&vertexData, &halfEdge, normals, vertCount);
            SmoothCurvatures(&vertexData, &halfEdge, vertCount);

            // Next, we flood-fill planar regions, and select the best regions
            NBest<cMaxPlanesPerSurface, PlaneData> bestPlanes;
            FloodFillLowCurvatureRegions(&vertexData, &halfEdge, normals, verts, vertCount, &bestPlanes);

            // and we then generate the plane equation
            XMMATRIX observerToSurface = XMMatrixInverse(nullptr, surfaceToObserver);
            XMVECTOR vUpInSurfaceSpace = XMVector3Normalize(XMVector3TransformNormal(cUpDirection, observerToSurface));
            GeneratePlaneEquations(&vertexData, &halfEdge, vertCount, verts, &bestPlanes, meshToMetersScale, snapToGravityThreshold, vUpInSurfaceSpace);

            // once we have plane equations, re-floodfill to generate our final set of vertices, and bounds
            // this can occur in a member when the data is being consumed
            FloodFillPlaneEquation(&vertexData, vertCount, &halfEdge, normals, verts, &bestPlanes, meshToMetersScale);

            vector<UINT32> vertexPlaneMapping = vector<UINT32>(vertCount);
            for (UINT32 i = 0; i < vertCount; ++i)
            {
                // we color vertices according to their coallesced vertex's color
                vertexPlaneMapping[i] = vertexData[halfEdge.m_spVertices[i] != nullptr ? halfEdge.m_spVertices[i]->vertex : i].plane;
            }

            // now that we have our "best" planes, create the WinRT objects that expose our data
            for (unsigned int i = 0; i < bestPlanes.num; ++i)
            {
                if (!bestPlanes.best[i].ShouldIgnorePlane())
                {
                    Plane planeEq;
                    XMFLOAT3 tangent;
                    bestPlanes.best[i].GetPlaneEquationData(&planeEq, &tangent);

                    XMVECTOR planeInObserverSpace = XMPlaneNormalize(TransformPlaneBetweenSpaces(planeEq.AsVector(), surfaceToObserver));
                    planeEq.StoreVector(planeInObserverSpace);

                    BoundingOrientedBox xmBoundsInMeshSpace = bestPlanes.best[i].GetBoundsInMeshSpace(verts, vertCount, vertexPlaneMapping.data());
                    BoundingOrientedBox xmBoundsInObserverSpace;
                    xmBoundsInMeshSpace.Transform(xmBoundsInObserverSpace, surfaceToObserver);

                    float area = GetArea(bestPlanes.best[i].GetPlaneId(), vertexPlaneMapping, numIndices, verts, indices);

                    // area is in mesh space - scale it to meters
                    area *= meshToMetersScale * meshToMetersScale;

                    planes.push_back({ planeEq, xmBoundsInObserverSpace, area });
                }
            }
        }

        return planes;
    }
}

#include "pch.h"
#include "PlaneFinding.h"
#include "Util.h"

using namespace DirectX;

namespace PlaneFinding
{
    const XMVECTOR cUpDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    struct PlaneGraphNode
    {
        BoundedPlane* plane;
        bool walked;
        vector<PlaneGraphNode*> neighbors;
    };

    float PlaneAngle(_In_ const Plane &p1, _In_ const Plane &p2)
    {
        return XMVectorGetX(XMVector3AngleBetweenVectors(p1.AsVector(), p2.AsVector()));
    }

    vector<PlaneGraphNode> BuildPlaneGraph(
        _In_ INT32 numPlanes,
        _In_count_(numPlanes) BoundedPlane* planes)
    {
        vector<PlaneGraphNode> nodes = vector<PlaneGraphNode>();

        // create PlaneGraphNodes for all the planes
        for (int i = 0; i < numPlanes; ++i)
        {
            nodes.push_back({ &planes[i], false });
        }

        // populate the neighbors
        for (UINT32 i = 0; i < nodes.size(); ++i)
        {
            for (UINT32 j = i + 1; j < nodes.size(); ++j)
            {
                if (PlaneAngle(nodes[i].plane->plane, nodes[j].plane->plane) < 0.3f && nodes[i].plane->bounds.Intersects(nodes[j].plane->bounds))
                {
                    nodes[i].neighbors.push_back(&nodes[j]);
                    nodes[j].neighbors.push_back(&nodes[i]);
                }
            }
        }

        return nodes;
    }

    vector<BoundedPlane*> GetNeighborsRecursive(PlaneGraphNode& startNode, vector<PlaneGraphNode>& nodes)
    {
        vector<BoundedPlane*> neighbors;

        queue<PlaneGraphNode*> toExpand;
        toExpand.push(&startNode);
        startNode.walked = true;

        while (!toExpand.empty())
        {
            PlaneGraphNode* expand = toExpand.front();
            toExpand.pop();
            neighbors.push_back(expand->plane);

            for (PlaneGraphNode* neighbor : expand->neighbors)
            {
                if (!neighbor->walked)
                {
                    neighbor->walked = true;
                    toExpand.push(neighbor);
                }
            }
        }

        return neighbors;
    }

    BoundingOrientedBox GetTightBounds(
        const vector<DirectX::XMFLOAT3> &bounds,
        const DirectX::XMVECTOR &plane,
        bool isGravityAligned)
    {
        // find tight bounding box
        UINT32 index = 0;
        XMVECTOR normal = XMVector3Normalize(XMVectorSetW(plane, 0));
        XMMATRIX planeToObserver;

        if (isGravityAligned)
        {
            // plane space is z=-normal, y=dominant tangent, x=orthogonal
            planeToObserver = XMMatrixIdentity();
            planeToObserver.r[2] = -normal; // negative normal, so if you are looking at the plane, the normal is pointing backwards
            planeToObserver.r[1] = XMVector3Normalize(XMVector3Cross(normal, cUpDirection));
            planeToObserver.r[0] = XMVector3Cross(planeToObserver.r[1], planeToObserver.r[2]);
        }
        else
        {
            planeToObserver = ComputeYAlignedRotation(-normal);
            // we actually want z-aligned instead of y-aligned (note that this incurs a flip in addition to rotation, so we need to negate an axis)
            std::swap(planeToObserver.r[1], planeToObserver.r[2]);
            planeToObserver.r[2] = -planeToObserver.r[2];
        }

        XMMATRIX observerToPlane = XMMatrixInverse(nullptr, planeToObserver);

        auto boundsInPlaneSpace = GetBoundsInOrientedSpace(!isGravityAligned, [&](XMFLOAT3* vert)
        {
            if (index < bounds.size())
            {
                XMStoreFloat3(vert, XMVector3TransformCoord(XMLoadFloat3(&bounds[index]), observerToPlane));
            }
            index++;
            return index <= bounds.size();
        });

        BoundingOrientedBox boundsInObserverSpace;
        boundsInPlaneSpace.Transform(boundsInObserverSpace, planeToObserver);
        return boundsInObserverSpace;
    }

    vector<BoundedPlane> MergePlanes(
        _In_ INT32 numSubPlanes,
        _In_count_(numSubPlanes) BoundedPlane* subPlanes,
        _In_ float minArea,
        _In_ float snapToGravityThreshold)
    {
        vector<PlaneGraphNode> nodes = BuildPlaneGraph(numSubPlanes, subPlanes);

        // find the cliques in our graph of PlaneGraphNodes
        // TODO: over large distances, low curvature walls will be handled as a single plane even though they may not be planar
        // consider whether that causes issues.  We can address the issue by deriving a plane equation and filtering verts again.

        // this is a breadth-first search to flood-fill the graph.  We expand each node only one time.
        vector<BoundedPlane> planes;
        for (auto &startNode : nodes)
        {
            if (!startNode.walked)
            {
                vector<BoundedPlane*> neighbors = GetNeighborsRecursive(startNode, nodes);

                // Compute aggregate area, center, normal, and the collection of vertices that define the bounding boxes for each of the planes
                // in this clique
                float totalArea = 0;
                vector<XMFLOAT3> boundVerts;
                XMVECTOR averageCenter = g_XMZero;
                XMVECTOR averageNormal = g_XMZero;
                for (BoundedPlane* boundedPlane : neighbors)
                {
                    // Rather than walk all the planes vertices again to re-run PCA, we average the plane equations of the component planes.
                    // This isn't guaranteed to give the plane through all the vertices if there are large angles between the planes.
                    // however, it saves compute time, and in practice our plane equations are close enough that this doesn't
                    // cause significant error.
                    XMVECTOR plane = XMPlaneNormalize(boundedPlane->plane.AsVector());

                    // We make a similar performance optimization for the bounding box - we find a tight bounding box that includes all the
                    // bounding boxes for each sub-plane.  We could make it tighter, but it would require walking all vertices again.
                    // Since the component bounding boxes are tight, the box that contains them is also relatively tight.
                    XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&boundedPlane->bounds.Orientation));
                    XMVECTOR dx = rotation.r[0] * boundedPlane->bounds.Extents.x;
                    XMVECTOR dy = rotation.r[1] * boundedPlane->bounds.Extents.y;
                    XMVECTOR dz = rotation.r[2] * boundedPlane->bounds.Extents.z;
                    XMVECTOR center = XMLoadFloat3(&boundedPlane->bounds.Center);

                    double area = boundedPlane->area;
                    totalArea += static_cast<float>(area);

                    for (int i = 0; i < 8; ++i)
                    {
                        float signx = i & 1 ? -1.0f : 1.0f;
                        float signy = i & 2 ? -1.0f : 1.0f;
                        float signz = i & 4 ? -1.0f : 1.0f;

                        XMFLOAT3 boundVert;
                        XMStoreFloat3(&boundVert, center + dx*signx + dy*signy + dz*signz);
                        boundVerts.push_back(boundVert);
                    }

                    // Project center of bounding box onto plane
                    center -= (XMPlaneDotCoord(plane, center) * plane);

                    // Add weighted normal and center to averages
                    averageCenter += static_cast<float>(area) * center;
                    averageNormal += static_cast<float>(area) * plane;
                }

                // If the total area is big enough, then create a merged plane for this clique
                if (totalArea > minArea)
                {
                    averageCenter /= totalArea;
                    averageNormal = XMVector3Normalize(averageNormal);
                    XMVECTOR averagePlane = XMPlaneFromPointNormal(averageCenter, averageNormal);
                    bool isGravityAligned = false;

                    if (snapToGravityThreshold != 0.0f)
                    {
                        Plane plane = Plane(averagePlane);
                        XMFLOAT3 center;

                        XMStoreFloat3(&center, averageCenter);

                        isGravityAligned = SnapToGravity(&plane, nullptr, center, snapToGravityThreshold, cUpDirection);

                        averagePlane = plane.AsVector();
                    }

                    Plane plane = Plane(averagePlane);

                    BoundingOrientedBox bounds = GetTightBounds(boundVerts, averagePlane, isGravityAligned);

                    planes.push_back({ plane, bounds, totalArea }); // add all our aggregated information for this clique to the surface observer plane, then return it
                }
            }
        }

        return planes;
    }
}
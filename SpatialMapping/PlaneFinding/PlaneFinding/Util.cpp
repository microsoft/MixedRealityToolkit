#include "pch.h"
#include "PlaneFinding.h"

using namespace DirectX;

namespace PlaneFinding
{
    const float ROTATING_CALIPERS_EPSILON = 0.01f; // a small epsilon value to handle rounding errors when calculating rotation angles

    vector<pair<XMFLOAT2, UINT32>> FindConvexHull(_In_ function<bool(XMFLOAT2*, UINT32*)> vertGenerator)
    {
        // We find a convex hull for a set of points (as defined by a callback function that iterates them) by
        // 1) generate the list of points
        // 2) sort them
        // 3) iterate along the sorted points to trace along the top/bottom of the convex hull
        // 4) for each vertex we add to the top/bottom, we need to look at the prior element added and check that it should
        //    be considered part of the convex hull.  This is determined based on the sign of the cross-product.

        // We use the Monotone Chain algorithm to calculate the convex-hull - http://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain,

        vector<pair<XMFLOAT2, UINT32>> planarVerts;

        XMFLOAT2 newVert;
        UINT32 index;
        while (vertGenerator(&newVert, &index))
        {
            planarVerts.push_back({ newVert, index });
        }

        // sort in the x-direction, and then if equal in the y-direction.  duplicate vertices will be removed by the isVertAbove/isVertBelow conditions below.
        std::sort(planarVerts.begin(), planarVerts.end(), [](const pair<XMFLOAT2, UINT32> &v1, const pair<XMFLOAT2, UINT32> &v2)
        {
            return v1.first.x < v2.first.x || (v1.first.x == v2.first.x && v1.first.y < v2.first.y);
        });

        // now that we have the sorted list of verts, look at them in order, and generate a convex hull
        vector<pair<XMFLOAT2, UINT32>> top;
        vector<pair<XMFLOAT2, UINT32>> bottom;

        // Given three points, return true if v is above line p1->p2, and false otherwise
        auto isVertAbove = [](const XMFLOAT2 &v, const XMFLOAT2 &p1, const XMFLOAT2 &p2)
        {
            XMFLOAT2 a = { p1.x - p2.x, p1.y - p2.y };
            XMFLOAT2 b = { v.x - p2.x, v.y - p2.y };
            return (a.x*b.y - b.x*a.y) < 0;
        };

        // Given three points, return true if v is below line p1->p2, and false otherwise
        auto isVertBelow = [](const XMFLOAT2 &v, const XMFLOAT2 &p1, const XMFLOAT2 &p2)
        {
            XMFLOAT2 a = { p1.x - p2.x, p1.y - p2.y };
            XMFLOAT2 b = { v.x - p2.x, v.y - p2.y };
            return (a.x*b.y - b.x*a.y) > 0;
        };
        for (auto const &vert : planarVerts)
        {
            // reduce top assuming we will add vert
            while (top.size() >= 2 && !isVertAbove(vert.first, top[top.size() - 1].first, top[top.size() - 2].first))
            {
                top.pop_back();
            }

            // reduce bottom assuming we will add vert
            while (bottom.size() >= 2 && !isVertBelow(vert.first, bottom[bottom.size() - 1].first, bottom[bottom.size() - 2].first))
            {
                bottom.pop_back();
            }

            top.push_back(vert);
            bottom.push_back(vert);
        }

        // Start by copying the top-list to our returned collection.
        vector<pair<XMFLOAT2, UINT32>> ret = move(top);

        ASSERT(bottom.size() >= 2);
        // We traced both top and bottom vertices in left-to-right order in one pass, so we need to reverse the order on the bottom
        // to make the convex hull a cycle.  We want to skip the first and last element of the list since they are already included in top.
        for (UINT32 i = 1; i < bottom.size() - 1; ++i)
        {
            ret.push_back(bottom[(bottom.size() - 1) - i]);
        }

        return ret;
    }

    DirectX::BoundingOrientedBox GetBoundsInOrientedSpace(_In_ bool findTightestBounds, _In_ function<bool(XMFLOAT3*)> vertGenerator)
    {
        // we find tight bounds by
        // 1. find the convex hull
        // 2. rotating calipers to find the ideal bounding box - http://en.wikipedia.org/wiki/Rotating_calipers

        // The idea behind rotating calipers is that we keep track of some extreme vertices, and slowly rotate our coordinate frame.
        // As we rotate, a vertex may no-longer be extreme in the new rotated coordinate frame, so we increment the index to the next vertex
        // in the convex hull that is now extreme.

        float zmin = FLT_MAX, zmax = -FLT_MAX;
        auto convexHull = FindConvexHull([&](XMFLOAT2 *planarVert, UINT32 *index) -> bool
        {
            *index = 0; // we dont' care about the index here - only useful when exposing the convex hull directly

            XMFLOAT3 vert;
            bool ret = vertGenerator(&vert);
            if (ret)
            {
                if (vert.z < zmin)
                {
                    zmin = vert.z;
                }

                if (vert.z > zmax)
                {
                    zmax = vert.z;
                }
            }
            *planarVert = { vert.x, vert.y };
            return ret;
        });

        // first we need to set up the calipers - extreme vertices that we will incrementally update as we rotate
        XMFLOAT2 maxv = convexHull[0].first;
        XMFLOAT2 minv = convexHull[0].first;
        struct RotatedBoundingBox
        {
            UINT32 maxx, maxy, minx, miny; // these represent the indices of the max/min x and y coordinates in a rotated coordated frame.
            float area;
            float minwidth;
            float angle;
        };

        // find the initial orientation's bounds:
        RotatedBoundingBox best = { 0, 0, 0, 0, FLT_MAX, FLT_MAX, XM_2PI };
        for (UINT32 i = 1; i < convexHull.size(); ++i)
        {
            const auto vertex = convexHull[i].first;
            if (vertex.x > maxv.x)
            {
                maxv.x = vertex.x;
                best.maxx = i;
            }
            else if (vertex.x < minv.x)
            {
                minv.x = vertex.x;
                best.minx = i;
            }

            if (vertex.y > maxv.y)
            {
                maxv.y = vertex.y;
                best.maxy = i;
            }
            else if (vertex.y < minv.y)
            {
                minv.y = vertex.y;
                best.miny = i;
            }
        }
        best.angle = 0;
        best.area = (maxv.x - minv.x) * (maxv.y - minv.y);
        best.minwidth = min(maxv.x - minv.x, maxv.y - minv.y);

        ASSERT(best.minx != best.maxx); // xmin and xmax indices should never be the same
        ASSERT(best.miny != best.maxy);

        ASSERT(best.minx <= best.maxy); // our vertices should be located around the convex hull xmin->ymax->xmax->ymin
        ASSERT(best.maxy <= best.maxx);
        ASSERT(best.maxx <= best.miny || best.minx == best.miny);
        ASSERT(best.miny <= best.minx + convexHull.size());

        ASSERT(best.minx <= convexHull.size()); // all of the indices should be in the convex hull
        ASSERT(best.maxx <= convexHull.size());
        ASSERT(best.miny <= convexHull.size());
        ASSERT(best.maxy <= convexHull.size());

        ASSERT(best.minx == 0); // we expect minx to be the first vertex in the convex hull

                                // Helper to calculate the rotation if we move from the given vertex to the next vertex in the convex hull
        auto getDeltaVectorForIndex = [&](UINT32 vert)
        {
            // return the delta between the given vertex and the subsequent vertex, so we can determine the angle our bounding box
            // would have to rotate to be parallel with this edge
            auto start = convexHull[vert % convexHull.size()].first;
            auto next = convexHull[(vert + 1) % convexHull.size()].first;

            ASSERT(start.x != next.x || start.y != next.y);

            return XMFLOAT2({ next.x - start.x, next.y - start.y });
        };

        // once we have the extreme vertices, we slowly rotate our coordinate system and adjust them
        // we rotate in such a way that only one extreme vertex changes at a time
        float angle = 0;
        RotatedBoundingBox current = best;

        BoundingOrientedBox bestBoxInPlaneSpace;
        bestBoxInPlaneSpace.Center = { (maxv.x + minv.x) / 2, (maxv.y + minv.y) / 2, (zmax + zmin) / 2 };
        bestBoxInPlaneSpace.Extents = { (maxv.x - minv.x) / 2, (maxv.y - minv.y) / 2, (zmax - zmin) / 2 };
        bestBoxInPlaneSpace.Orientation = { 0, 0, 0, 1 };

        if (findTightestBounds)
        {
            RotatedBoundingBox initial = best;

            // The tightest bounding box will share a side with the convex hull.  We start with a candidate bounding box oriented along
            // the x/y axes and iterate through all orientations where the box is aligned with an edge of the convex hull.  The maximum possible rotations
            // we need to consider is convexHull.size(), which would be a rotation of 90 degrees.
            // Each iteration through the loop, we pick the vertex from our extreme vertices that has the smallest incremental rotation along its outgoing edge.
            // A neat trick is that the other extreme vertices remain extreme in the new rotated orientation.
            while (angle <= XM_PIDIV2 + ROTATING_CALIPERS_EPSILON &&
                current.minx <= initial.maxy &&
                current.maxy <= initial.maxx &&
                current.maxx <= initial.miny &&
                current.miny <= convexHull.size())
            {
                const auto vectForXmin = getDeltaVectorForIndex(current.minx);
                const auto vectForXmax = getDeltaVectorForIndex(current.maxx);
                const auto vectForYmin = getDeltaVectorForIndex(current.miny);
                const auto vectForYmax = getDeltaVectorForIndex(current.maxy);

                UINT32* boundIndices[4] = { &current.minx, &current.maxx, &current.miny, &current.maxy };
                float angles[4] = {
                    atan2(vectForXmin.x, vectForXmin.y),
                    atan2(-vectForXmax.x, -vectForXmax.y),
                    atan2(vectForYmin.y, -vectForYmin.x),
                    atan2(-vectForYmax.y, vectForYmax.x) };

                int index = 0;
                float minAngle = XM_PI * 4 + angle;
                for (int i = 0; i < 4; ++i)
                {
                    if (angles[i] > -ROTATING_CALIPERS_EPSILON && angles[i] < 0)
                    {
                        // the vector between vertices are horizontal/vertical, so angle is close to 0, treat it as zero
                        // this can only occur with a rounding error
                        angles[i] = 0;
                    }
                    else if (angles[i] < 0)
                    {
                        angles[i] += XM_PI * 2;
                    }

                    if (angles[i] < minAngle)
                    {
                        minAngle = angles[i];
                        index = i;
                    }
                }

                *(boundIndices[index]) = ((*(boundIndices[index])) + 1);

                ASSERT(current.minx <= current.maxy); // we should remain ordering of vertices xmin->ymax->xmax->ymin as we rotate
                ASSERT(current.maxy <= current.maxx);
                ASSERT(current.maxx <= current.miny || best.minx == best.miny);
                ASSERT(current.miny <= current.minx + convexHull.size());

                ASSERT(current.minx != current.maxx); // and we shouldn't ever have min and max indices equal
                ASSERT(current.miny != current.maxy);


                // now update our box:
                angle = minAngle;
                current.angle = minAngle;
                if (angle < XM_PIDIV2 + ROTATING_CALIPERS_EPSILON)
                {
                    XMVECTOR vertsInRotatedPlaneSpace[4];
                    const XMMATRIX rotationTransform = XMMatrixRotationZ(angle);
                    for (int i = 0; i < 4; ++i)
                    {
                        vertsInRotatedPlaneSpace[i] = XMVector3TransformCoord(XMLoadFloat2(&convexHull[(*(boundIndices[i])) % convexHull.size()].first), rotationTransform);
                    }

                    BoundingOrientedBox xmBoundsInPlaneSpace;
                    xmBoundsInPlaneSpace.Center = { XMVectorGetX(vertsInRotatedPlaneSpace[0] + vertsInRotatedPlaneSpace[1]) / 2, XMVectorGetY(vertsInRotatedPlaneSpace[2] + vertsInRotatedPlaneSpace[3]) / 2, (zmax + zmin) / 2 };
                    xmBoundsInPlaneSpace.Extents = { XMVectorGetX(vertsInRotatedPlaneSpace[1] - vertsInRotatedPlaneSpace[0]) / 2, XMVectorGetY(vertsInRotatedPlaneSpace[3] - vertsInRotatedPlaneSpace[2]) / 2, (zmax - zmin) / 2 };
                    xmBoundsInPlaneSpace.Orientation = { 0, 0, 0, 1 };
                    xmBoundsInPlaneSpace.Transform(xmBoundsInPlaneSpace, XMMatrixTranspose(rotationTransform)); // rotate back to plane space from rotated plane space

                    const XMFLOAT2 size = { xmBoundsInPlaneSpace.Extents.x * 2, xmBoundsInPlaneSpace.Extents.y * 2 };
                    current.area = size.x * size.y;
                    current.minwidth = min(size.x, size.y);
                    if (current.area < best.area || (current.area == best.area && current.minwidth < best.minwidth))
                    {
                        best = current;
                        bestBoxInPlaneSpace = xmBoundsInPlaneSpace;
                    }
                }
            }
        }

        return bestBoxInPlaneSpace;
    }

    bool SnapToGravity(_Inout_ Plane* plane, _Inout_opt_ XMFLOAT3* tangent, _In_ const XMFLOAT3& center, float snapToGravityThreshold, _In_ const XMVECTOR& vUp)
    {
        XMVECTOR vNormal = XMLoadFloat3(&plane->normal);
        XMVECTOR vCenter = XMLoadFloat3(&center);

        float dotGravity = XMVectorGetX(XMVector3Dot(vNormal, vUp));
        float dotProductThreshold = cosf(XMConvertToRadians(snapToGravityThreshold));
        bool isGravityAligned = false;

        // check for nearly horizontal planes
        if (dotGravity > dotProductThreshold)
        {
            vNormal = vUp;
        }
        else if (dotGravity < -dotProductThreshold)
        {
            vNormal = -vUp;
        }
        else
        {
            // check for nearly vertical planes
            XMVECTOR vNormalProjectedPerpendicularToGravity = vNormal - (vUp * dotGravity);
            float dotPerpendicularToGravity = XMVectorGetX(XMVector3Length(vNormalProjectedPerpendicularToGravity));
            if (fabs(dotPerpendicularToGravity) > dotProductThreshold)
            {
                vNormal = XMVector3Normalize(vNormalProjectedPerpendicularToGravity);
                isGravityAligned = true;
            }
            else
            {
                // plane should not be snapped, so exit without modifying plane/tangent
                return false;
            }
        }

        // update the plane equation
        plane->StoreVector(XMPlaneFromPointNormal(vCenter, vNormal));

        // update the tangent vector
        if (tangent != nullptr)
        {
            XMVECTOR vTangent = (isGravityAligned)
                ? XMVector3Cross(vNormal, vUp)
                : XMVector3Cross(XMVector3Cross(vNormal, XMLoadFloat3(tangent)), vNormal);

            XMStoreFloat3(tangent, XMVector3Normalize(vTangent));
        }

        return isGravityAligned;
    }
}
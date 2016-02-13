#pragma once

namespace PlaneFinding
{

#pragma pack(push, 1)

    struct MeshData
    {
        DirectX::XMFLOAT4X4 transform;
        INT32 vertCount;
        INT32 indexCount;
        DirectX::XMFLOAT3* verts;
        DirectX::XMFLOAT3* normals;
        INT32* indices;
    };

    struct Plane
    {
        DirectX::XMFLOAT3 normal;
        FLOAT d;

        Plane() {}
        Plane(const DirectX::XMFLOAT3& normal, FLOAT d) : normal(normal), d(d) {}
        Plane(const DirectX::XMVECTOR& vec) { StoreVector(vec); }

        void StoreVector(const DirectX::XMVECTOR& vec)
        {
            XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vec);
        }

        const DirectX::XMVECTOR AsVector() const
        {
            return XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this));
        }
    };

    struct BoundedPlane
    {
        Plane plane;
        DirectX::BoundingOrientedBox bounds;
        FLOAT area;
    };

#pragma pack(pop)

    vector<BoundedPlane> FindPlanes(
        _In_ INT32 numMeshes,
        _In_count_(numMeshes) MeshData* meshes,
        _In_ float snapToGravityThreshold);

    vector<BoundedPlane> MergePlanes(
        _In_ INT32 numSubPlanes,
        _In_count_(numSubPlanes) BoundedPlane* subPlanes,
        _In_ float minArea,
        _In_ float snapToGravityThreshold);
}

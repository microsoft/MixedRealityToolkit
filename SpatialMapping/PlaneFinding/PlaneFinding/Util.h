#pragma once

namespace PlaneFinding
{
    DirectX::BoundingOrientedBox GetBoundsInOrientedSpace(
        _In_ bool findTightestBounds,
        _In_ function<bool(DirectX::XMFLOAT3*)> vertGenerator);

    bool SnapToGravity(
        _Inout_ Plane* plane,
        _Inout_opt_ DirectX::XMFLOAT3* tangent,
        _In_ const DirectX::XMFLOAT3& center,
        _In_ float snapToGravityThreshold,
        _In_ const DirectX::XMVECTOR& vUp);
}
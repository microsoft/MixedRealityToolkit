#pragma once

#include <DirectXMath.h>

#define VERTICES_PER_TRIANGLE 3

static const float XMEpsilon = 1e-3f;

static const DirectX::XMFLOAT4X4 XMFloat4x4Zero(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0);

static const DirectX::XMFLOAT4X3 XMFloat4x3Identity(
    1, 0, 0, 
    0, 1, 0, 
    0, 0, 1, 
    0, 0, 0);

static const DirectX::XMFLOAT4X4 XMFloat4x4Identity(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);

static const DirectX::XMFLOAT4X4 XMFloat4x4YFlip(
    1, 0, 0, 0,
    0,-1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);

static const DirectX::XMFLOAT4X4 XMFloat4x4ZFlip(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0,-1, 0,
    0, 0, 0, 1);

static const DirectX::XMFLOAT4X4 XMFloat4x4YZFlip(
    1, 0, 0, 0,
    0,-1, 0, 0,
    0, 0,-1, 0,
    0, 0, 0, 1);

static const DirectX::XMUINT2  XMUZero(0, 0);
static const DirectX::XMFLOAT3 XMZero(0, 0, 0);
static const DirectX::XMFLOAT3 XMForward(0, 0, -1);
static const DirectX::XMFLOAT3 XMBack(0, 0, 1);
static const DirectX::XMFLOAT3 XMUp(0, 1, 0);
static const DirectX::XMFLOAT3 XMDown(0, -1, 0);
static const DirectX::XMFLOAT3 XMRight(1, 0, 0);
static const DirectX::XMFLOAT3 XMLeft(-1, 0, 0);
static const DirectX::XMFLOAT4 XMHomogeneousZero(0, 0, 0, 1);
static const DirectX::XMFLOAT4 XMRotationIdentity(0, 0, 0, 1);

static inline bool operator==(const DirectX::XMMATRIX& xmLeft, const DirectX::XMMATRIX& xmRight)
{
    const DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(XMEpsilon);

    return
        (DirectX::XMVector4NearEqual(xmLeft.r[0], xmRight.r[0], epsilon) &&
        DirectX::XMVector4NearEqual(xmLeft.r[1], xmRight.r[1], epsilon) &&
        DirectX::XMVector4NearEqual(xmLeft.r[2], xmRight.r[2], epsilon) &&
        DirectX::XMVector4NearEqual(xmLeft.r[3], xmRight.r[3], epsilon));
}

static inline bool operator==(const DirectX::XMFLOAT4X4& left, const DirectX::XMFLOAT4X4& right)
{
    const DirectX::XMMATRIX xmLeft = DirectX::XMLoadFloat4x4(&left);
    const DirectX::XMMATRIX xmRight = DirectX::XMLoadFloat4x4(&right);

    return xmLeft == xmRight;
}

static inline bool operator==(const DirectX::XMFLOAT3& left, const DirectX::XMFLOAT3& right)
{
    const DirectX::XMVECTOR xmLeft = DirectX::XMLoadFloat3(&left);
    const DirectX::XMVECTOR xmRight = DirectX::XMLoadFloat3(&right);

    const DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(XMEpsilon);

    return DirectX::XMVector4NearEqual(xmLeft, xmRight, epsilon);
}

inline bool AreEqual(const DirectX::XMFLOAT3& left, const DirectX::XMFLOAT3& right, const float xmEpsilon)
{
    const DirectX::XMVECTOR xmLeft = DirectX::XMLoadFloat3(&left);
    const DirectX::XMVECTOR xmRight = DirectX::XMLoadFloat3(&right);

    const DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(xmEpsilon);

    return DirectX::XMVector4NearEqual(xmLeft, xmRight, epsilon);
}

static inline bool operator!=(const DirectX::XMFLOAT3& left, const DirectX::XMFLOAT3& right)
{
    return !(left == right);
}


static inline bool operator==(const DirectX::XMFLOAT4& left, const DirectX::XMFLOAT4& right)
{
    const DirectX::XMVECTOR xmLeft = DirectX::XMLoadFloat4(&left);
    const DirectX::XMVECTOR xmRight = DirectX::XMLoadFloat4(&right);

    const DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(XMEpsilon);

    return DirectX::XMVector4NearEqual(xmLeft, xmRight, epsilon);
}

static inline bool operator==(const DirectX::XMVECTOR& left, const DirectX::XMVECTOR& right)
{
    const DirectX::XMVECTOR epsilon = DirectX::XMVectorReplicate(XMEpsilon);

    return DirectX::XMVector4NearEqual(left, right, epsilon);
}

inline DirectX::XMMATRIX ComputeYAlignedRotation(_In_ const DirectX::XMVECTOR& yVector)
{
    using namespace DirectX;
    XMFLOAT3 xmGrav;
    XMStoreFloat3(&xmGrav, yVector);
    XMVECTOR ortho = XMVectorSet(1.0f - xmGrav.x * xmGrav.x, -xmGrav.x * xmGrav.y, -xmGrav.x * xmGrav.z, 0);
    
    XMMATRIX result = XMMatrixIdentity();
    result.r[1] = -yVector;

    float orthoLength;
    XMStoreFloat(&orthoLength, XMVector3Length(ortho));
    if (orthoLength > XMEpsilon)
    {
        result.r[0] = XMVector3Normalize(ortho);
        result.r[2] = XMVector3Cross(result.r[0], result.r[1]);
    }
    else
    {
        ortho = XMVectorSet(-xmGrav.z * xmGrav.x, -xmGrav.z * xmGrav.y, 1.0f - xmGrav.z * xmGrav.z, 0);
        result.r[2] = XMVectorScale(XMVector3Normalize(ortho), -1);
        result.r[0] = XMVector3Cross(result.r[2], result.r[1]);
    }

    return result;
}

inline DirectX::XMVECTOR TransformPlaneBetweenSpaces(_In_ const DirectX::XMVECTOR& planeInA, _In_ const DirectX::XMMATRIX& aToB)
{
    using namespace DirectX; // pick up functions and operators
    auto const noramlizedPlaneInA = XMPlaneNormalize(planeInA);
    auto const pointInB = XMVector3TransformCoord(-noramlizedPlaneInA * XMVectorSplatW(noramlizedPlaneInA), aToB);
    auto const normalInB = XMVector3TransformNormal(noramlizedPlaneInA, aToB);
    return XMPlaneFromPointNormal(pointInB, normalInB);
}
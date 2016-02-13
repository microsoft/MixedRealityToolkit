#include "pch.h"
#include "PlaneFinding.h"
#include "PCAHelper.h"

using namespace DirectX;

namespace PlaneFinding
{
    void PCAHelper::AddVertex(XMFLOAT3 vert)
    {
        const float x = vert.x - m_mean.x;
        const float y = vert.y - m_mean.y;
        const float z = vert.z - m_mean.z;
        m_xx += x*x;
        m_xy += x*y;
        m_xz += x*z;
        m_yy += y*y;
        m_yz += y*z;
        m_zz += z*z;
    }

    Plane PCAHelper::GetPlaneEquation()
    {
        const XMVECTOR normal = XMLoadFloat3(&m_normal);
        const XMVECTOR mean = XMLoadFloat3(&m_mean);
        const float D = -XMVectorGetX(XMVector3Dot(normal, mean));
        const Plane ret = { { m_normal.x, m_normal.y, m_normal.z }, D };
        return ret;
    }

    void PCAHelper::Solve()
    {
        XMFLOAT3X3 mat = {
            m_xx, m_xy, m_xz,
            m_xy, m_yy, m_yz,
            m_xz, m_yz, m_zz
        };

        FindEigenvectors(mat, &m_tangent, &m_cotangent, &m_normal);

        // Special case handling for the scenario where the points are perfectly planar
        if (m_normal == XMZero)
        {
            XMStoreFloat3(&m_normal, XMVector3Cross(XMLoadFloat3(&m_tangent), XMLoadFloat3(&m_cotangent)));
        }
    }

    void PCAHelper::MakeTridiagonal(XMFLOAT3X3 M, _Out_ XMFLOAT3X3 *Q, _Out_ XMFLOAT3X3 *T)
    {
        // Input: M is a symmetric matrix
        // output: Q is an orthogonal matrix, and T a triagonal symmetric matrix such that Qt*T*Q=M
        if (M._31 == 0)
        {
            // we are already tridiagonal
            *Q = { 1, 0, 0,
                0, 1, 0,
                0, 0, 1 };
            *T = M;
        }
        else
        {
            // We create a Givens rotation matrix for the similarity transform.
            // See this wikipedia page for an explanation: http://en.wikipedia.org/wiki/Givens_rotation#Triangularization
            float theta = atan2f(-M._31, M._21);
            XMFLOAT3X3 G = { 1, 0, 0,
                0, cos(theta), -sin(theta),
                0, sin(theta), cos(theta) };

            XMMATRIX xmG = XMLoadFloat3x3(&G);
            XMStoreFloat3x3(Q, XMMatrixTranspose(xmG));
            XMStoreFloat3x3(T, xmG * XMLoadFloat3x3(&M) * XMMatrixTranspose(xmG)); // R
        }
    }

    void PCAHelper::QRDecomposition(XMFLOAT3X3 M, _Out_ XMFLOAT3X3 *Q, _Out_ XMFLOAT3X3 *R)
    {
        // Input: M is a tridiagonal symmetric matrix
        // Output: Q is an orthogonal matrix, and R is an upper triangular matrix so that A=QR.

        // we use the gram-schmidt process to calculate Q:
        //      a b 0
        // M =  b c d
        //      0 d e

        // the first column is the same as M, but normalized
        const XMVECTORF32 a1 = { M._11, M._12, 0 };
        const XMVECTOR u1 = a1;
        const XMVECTOR e1 = XMVector3Normalize(u1);

        // the second column is made orthogonal, by subtracting out the projection onto the first column vector
        const XMVECTORF32 a2 = { M._21, M._22, M._23 };
        const XMVECTOR u2 = a2 - e1 * XMVector3Dot(e1, a2);
        const XMVECTOR e2 = XMVector3Normalize(u2);

        // the third column is again orthogonal by subtracting out the first two columns
        const XMVECTORF32 a3 = { 0, M._32, M._33 };
        const XMVECTOR u3 = a3 - e1 * XMVector3Dot(e1, a3) - e2 * XMVector3Dot(e2, a3);
        const XMVECTOR e3 = XMVector3Normalize(u3);

        XMMATRIX Qt = XMMatrixIdentity();
        Qt.r[0] = e1;
        Qt.r[1] = e2;
        Qt.r[2] = e3;
        XMStoreFloat3x3(Q, XMMatrixTranspose(Qt));

        // R=Qt * M
        XMStoreFloat3x3(R, Qt * XMLoadFloat3x3(&M));
    }

    void PCAHelper::FindEigenvectors(_In_ const XMFLOAT3X3 &M, _Out_ XMFLOAT3 *eig1, _Out_ XMFLOAT3 *eig2, _Out_ XMFLOAT3 *eig3)
    {
        // Input: M is a symmetric matrix
        // Output: eigenvalues and eigenvectors of M
        XMFLOAT3X3 Q;
        XMFLOAT3X3 A;
        MakeTridiagonal(M, &Q, &A);
        XMMATRIX QTotal = XMLoadFloat3x3(&Q);

        for (int i = 0; i < 32; ++i)
        {
            XMFLOAT3X3 R;
            QRDecomposition(A, &Q, &R);
            XMStoreFloat3x3(&A, XMLoadFloat3x3(&R)*XMLoadFloat3x3(&Q));
            QTotal = QTotal * XMLoadFloat3x3(&Q);
        }

        // eigenvalues are diagonal entries of A
        // but we aren't consuming them, so aren't exposing them

        // eigen vectors are columns of QTotal
        const XMMATRIX Qt = XMMatrixTranspose(QTotal);
        XMStoreFloat3(eig1, Qt.r[0]);
        XMStoreFloat3(eig2, Qt.r[1]);
        XMStoreFloat3(eig3, Qt.r[2]);

        eigenValues.x = A._11;
        eigenValues.y = A._22;
        eigenValues.z = A._33;
    }
}
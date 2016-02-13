#pragma once

namespace PlaneFinding
{
    class PCAHelper
    {
    private:
        // the covariance matrix:
        // E((x - avg(x))*(x - avg(x)))     E((y - avg(y))*(x - avg(x)))     E((z - avg(z))*(x - avg(x)))
        // E((x - avg(x))*(y - avg(y)))     E((y - avg(y))*(y - avg(y)))     E((z - avg(z))*(y - avg(y)))
        // E((x - avg(x))*(z - avg(z)))     E((y - avg(y))*(z - avg(z)))     E((z - avg(z))*(z - avg(z)))
        // The eigenvectors of this are the principal components.
        // Since This matrix is symmetrix, we only need the upper left corner.
        float m_xx = 0;
        float m_xy = 0;
        float m_xz = 0;
        float m_yy = 0;
        float m_yz = 0;
        float m_zz = 0;

        // we need to know the average x/y/z to calculate the covariance matrix when adding points
        DirectX::XMFLOAT3 m_mean;

        DirectX::XMFLOAT3 m_normal;
        DirectX::XMFLOAT3 m_tangent;
        DirectX::XMFLOAT3 m_cotangent;
        DirectX::XMFLOAT3 eigenValues;

    public:
        void SetMean(DirectX::XMFLOAT3 m)
        {
            m_mean = m;
        }

        void AddVertex(DirectX::XMFLOAT3 vert);

        Plane GetPlaneEquation();

        DirectX::XMFLOAT3 GetStandardDeviations()
        {
            return eigenValues;
        }

        DirectX::XMFLOAT3 GetTangent()
        {
            return m_tangent;
        }

        void Solve();

    private:
        void MakeTridiagonal(DirectX::XMFLOAT3X3 M, _Out_ DirectX::XMFLOAT3X3 *Q, _Out_ DirectX::XMFLOAT3X3 *T);
        void QRDecomposition(DirectX::XMFLOAT3X3 M, _Out_ DirectX::XMFLOAT3X3 *Q, _Out_ DirectX::XMFLOAT3X3 *R);
        void FindEigenvectors(_In_ const DirectX::XMFLOAT3X3 &M, _Out_ DirectX::XMFLOAT3 *eig1, _Out_ DirectX::XMFLOAT3 *eig2, _Out_ DirectX::XMFLOAT3 *eig3);
    };
}
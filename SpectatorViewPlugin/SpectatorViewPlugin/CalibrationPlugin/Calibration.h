#pragma once

namespace CalibrationPlugin
{
    namespace Native
    {
        public ref class ArUcoCorners sealed
        {
        public:
            property int markerId;
            property Windows::Foundation::Numerics::float3 topLeft;
            property Windows::Foundation::Numerics::float3 topRight;
            property Windows::Foundation::Numerics::float3 bottomLeft;
            property Windows::Foundation::Numerics::float3 bottomRight;
        };

        public ref class UnityCameraOrientation sealed
        {
        public:
            property Windows::Foundation::Numerics::float3 position;
            property Windows::Foundation::Numerics::quaternion rotation;
        };

        public ref class Intrinsics sealed
        {
        public:
            property double reprojectionError;
            property Windows::Foundation::Numerics::float2 focalLength;
            property Windows::Foundation::Numerics::float2 principalPoint;
            property Windows::Foundation::Numerics::float3 radialDistortion;
            property Windows::Foundation::Numerics::float2 tangentialDistortion;
        };

        public ref class MultiIntrinsics sealed
        {
        public:
            property Intrinsics^ Standard;
            property Intrinsics^ PreCalcMatrix;
            property Intrinsics^ StandardPositionCorrected;
            property Intrinsics^ PreCalcPositionCorrected;
        };

        public ref class Extrinsics sealed
        {
        public:
            property Windows::Foundation::Numerics::float4x4 extrinsics;
        };

        public ref class Calibration sealed
        {
        public:
            Calibration() {}
            bool ProcessImage(
                const Platform::Array<byte>^ image,
                int imageWidth,
                int imageHeight,
                const Platform::Array<ArUcoCorners^>^ worldCorners,
                UnityCameraOrientation^ orientation);
            MultiIntrinsics^ ProcessIntrinsics();

        private:
            Intrinsics^ CreateIntrinsics(
                double reprojectionError,
                cv::Mat cameraMat,
                cv::Mat distCoeff);

            int width = 0;
            int height = 0;
            std::vector<std::vector<cv::Vec3f>> worldPointObservations;
            std::vector<std::vector<cv::Vec2f>> imagePointObservations;
        };
    }
}

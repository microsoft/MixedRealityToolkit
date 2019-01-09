////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\EngineCommon.h>

namespace Neso 
{
    // Helpers to extract data from transform matrices
    namespace float4x4_util
    {
        using float3 = winrt::Windows::Foundation::Numerics::float3;
        using quaternion = winrt::Windows::Foundation::Numerics::quaternion;
        using float4x4 = winrt::Windows::Foundation::Numerics::float4x4;

        inline float3 right(const float4x4& transform)
        {
            return { +transform.m11, +transform.m12, +transform.m13 };
        }

        inline float3 left(const float4x4& transform)
        {
            return { -transform.m11, -transform.m12, -transform.m13 };
        }

        inline float3 up(const float4x4& transform)
        {
            return { +transform.m21, +transform.m22, +transform.m23 };
        }

        inline float3 down(const float4x4& transform)
        {
            return { -transform.m21, -transform.m22, -transform.m23 };
        }

        inline float3 backward(const float4x4& transform)
        {
            return { +transform.m31, +transform.m32, +transform.m33 };
        }

        inline float3 forward(const float4x4& transform)
        {
            return { -transform.m31, -transform.m32, -transform.m33 };
        }

        inline float3 position(const float4x4& transform)
        {
            return { +transform.m41, +transform.m42, +transform.m43 };
        }

        inline quaternion orientation(const float4x4& transform)
        {
            return winrt::Windows::Foundation::Numerics::make_quaternion_from_rotation_matrix(transform);
        }

        inline float4x4 inverse(const float4x4& transform)
        {
            float4x4 out;
            fail_fast_if(!invert(transform, &out));
            return out;
        }

        inline float4x4 remove_scale(const float4x4& transform) 
        {
            using namespace winrt::Windows::Foundation::Numerics;
            quaternion rotation;
            float3 scale, translation;
            fail_fast_if(!decompose(transform, &scale, &rotation, &translation));
            return make_float4x4_from_quaternion(rotation) * make_float4x4_translation(translation);
        }
    }

    namespace location_util 
    {
        using float3 = winrt::Windows::Foundation::Numerics::float3;
        using quaternion = winrt::Windows::Foundation::Numerics::quaternion;
        using float4x4 = winrt::Windows::Foundation::Numerics::float4x4;
        using SpatialInteractionSourceLocation = winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceLocation;

        inline float3 position(const SpatialInteractionSourceLocation& location)
        {
            return (location.Position()) ? location.Position().Value() : float3::zero();
        }

        inline quaternion orientation(const SpatialInteractionSourceLocation& location)
        {
            return (location.Orientation()) ? location.Orientation().Value() : quaternion::identity();
        }

        inline float4x4 matrix(const SpatialInteractionSourceLocation& location)
        {
            using namespace winrt::Windows::Foundation::Numerics;
            return make_float4x4_from_quaternion(orientation(location)) * make_float4x4_translation(position(location));
        }
    }
}


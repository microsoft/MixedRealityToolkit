#pragma once

#include <winrt\Windows.UI.Input.Spatial.h>

namespace SpatialInputUtilities
{
    static inline std::optional<winrt::Windows::Foundation::Numerics::float3> GetVelocityNearSourceLocation(
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceLocation const& sourceLocation,
        winrt::Windows::Foundation::Numerics::float3 const& positionNearSourceLocation)
    {
        using namespace winrt::Windows::Foundation;
        using namespace winrt::Windows::Foundation::Numerics;

        const IReference<float3> graspVelocity = sourceLocation.Velocity();
        const IReference<float3> graspPosition = sourceLocation.Position();
        const IReference<float3> angularVelocity = sourceLocation.AngularVelocity();

        if (!graspVelocity || !graspPosition || !angularVelocity)
        {
            return {};
        }

        // Compute the tangential velocity at positionNearSourceLocation due to angular velocity.
        const float3 positionNearSourceLocationOffset = positionNearSourceLocation - graspPosition.Value();
        const float3 angularTangentialVelocity = cross(angularVelocity.Value(), positionNearSourceLocationOffset);

        // Combine the tangential velocity with the velocity to get the combined velocity.
        return graspVelocity.Value() + angularTangentialVelocity;
    }
}
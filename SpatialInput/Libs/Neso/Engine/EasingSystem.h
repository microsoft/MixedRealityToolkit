#pragma once

#include <Neso\Engine\Engine.h>

namespace Neso
{
    struct Easing : Component<Easing>
    {
        winrt::Windows::Foundation::Numerics::float3 TargetPosition{ 0,0,0 };
        winrt::Windows::Foundation::Numerics::quaternion TargetOrientation = winrt::Windows::Foundation::Numerics::quaternion::identity();
        float PositionEasingFactor = 0;
        float OrientationEasingFactor = 0;
    };

    class EasingSystem : public System<EasingSystem>
    {
    public:
        using System::System;

    protected:
        void Update(float) override;
    };
}
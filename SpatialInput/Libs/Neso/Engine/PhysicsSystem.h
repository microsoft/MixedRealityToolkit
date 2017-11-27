#pragma once

#include <Neso\Engine\Engine.h>

namespace Neso {
    class PhysicsSystem : public System<PhysicsSystem>
    {
    public:
        using System::System;

        static const winrt::Windows::Foundation::Numerics::float3 EarthGravity;

    protected:
        void Update(float dt) override;
    };
}
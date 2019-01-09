////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>

namespace Neso 
{
    ////////////////////////////////////////////////////////////////////////////////
    // PhysicsSystem
    // Simple physics system to move objects around with basic integration (acceleration, velocity)
    class PhysicsSystem : public System<PhysicsSystem>
    {
    public:
        using System::System;

        static const winrt::Windows::Foundation::Numerics::float3 EarthGravity;

    protected:
        void Update(float dt) override;
    };
}

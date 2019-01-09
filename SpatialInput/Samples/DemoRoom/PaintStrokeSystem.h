////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Pbr\PbrPrimitive.h>

namespace DemoRoom 
{
    struct PaintStrokeComponent : Neso::Component<PaintStrokeComponent>
    {
        struct Square
        {
            winrt::Windows::Foundation::Numerics::float3 TopLeft, TopRight, BottomLeft, BottomRight;
        };

        std::vector<Square> squares;
        bool strokeChanged{ true };

        void AddPoint(const winrt::Windows::Foundation::Numerics::float4x4& transformationMatrix, float width);
        Pbr::PrimitiveBuilder GetPrimitiveData();
    };

    ////////////////////////////////////////////////////////////////////////////////
    // PaintStrokeSystem
    // This System manages the PaintStrokeComponents and automatically generates the 3D mesh for each stroke
    class PaintStrokeSystem : public Neso::System<PaintStrokeSystem>
    {
    public:
        PaintStrokeSystem(Neso::Engine& core, std::shared_ptr<Pbr::Resources> pbrResources);
        ~PaintStrokeSystem() = default;

    protected:
        void Update(float) override;

    private:
        std::shared_ptr<Pbr::Resources> m_pbrResources{ nullptr };
    };

}

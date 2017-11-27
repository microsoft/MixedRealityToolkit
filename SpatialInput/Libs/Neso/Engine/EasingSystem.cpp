#include "pch.h"
#include "EasingSystem.h"
#include "EntityStore.h"
#include "CommonComponents.h"

using namespace Neso;
using namespace winrt::Windows::Foundation::Numerics;

void EasingSystem::Update(float dt)
{
    for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<Transform, Easing>())
    {
        auto transform = std::get<Transform*>(componentSet);
        auto easing = std::get<Easing*>(componentSet);

        transform->position = lerp(transform->position, easing->TargetPosition, easing->PositionEasingFactor);
        transform->orientation = slerp(transform->orientation, easing->TargetOrientation, easing->OrientationEasingFactor);
    }
}
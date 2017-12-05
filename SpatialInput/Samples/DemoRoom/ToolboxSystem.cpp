////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "ToolboxSystem.h"
#include "MotionControllerSystem.h"

#include <Neso\Engine\EasingSystem.h>
#include <Neso\Engine\PbrModelCache.h>
#include <Neso\Engine\CommonComponents.h>
#include <Neso\Engine\HolographicScene.h>

using namespace Neso;
using namespace DemoRoom;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

struct MotionControllerPrefab : EntityPrefab<Transform, PbrRenderable, MotionControllerComponent, ToolComponent>
{};

struct TextDisplay : EntityPrefab<Transform, TextRenderable>
{};

static const std::wstring_view InstructionalText = L"Press the menu button to bring interaction objects toward you.\n\nGrasp (grasp button) an interaction object to use it.";

namespace 
{
    bool HitTest(float3 positionA, float3 positionB, float diameter)
    {
        auto distance = length(positionA - positionB);
        return distance < diameter;
    }
}

void ToolboxSystem::AddToolSystem(std::shared_ptr<ToolSystemBase> system)
{
    system->Register({ m_controllers[Left].Controller, m_controllers[Right].Controller });

    m_selectorObjects.emplace(system->type(), system->CreateToolSelector());

    m_selectors.emplace(system->type(), system);
    
    for (auto& context : m_controllers)
    {
        SwitchToolType(*context.Controller, system->type());
    }
}

void ToolboxSystem::RemoveToolSystem(const std::shared_ptr<ToolSystemBase>& system)
{
    m_selectors.erase(system->type());

    m_selectorObjects.erase(system->type());

    system->Unregister();
}

void ToolboxSystem::Start()
{
    auto m_entityStore = m_engine.Get<EntityStore>();

    for (size_t i = 0; i < m_controllers.size(); ++i)
    {
        const ControllerHand hand = static_cast<ControllerHand>(i);

        m_controllers[i].Hand = hand;
        m_controllers[i].Controller = m_entityStore->Create<MotionControllerPrefab>();
        m_controllers[i].Controller->Get<MotionControllerComponent>()->requestedHandedness = ControllerHandToHandedness(hand);
        m_controllers[i].Controller->Get<MotionControllerComponent>()->attachControllerModel = true;
    }

    m_instructionalText = m_entityStore->Create<TextDisplay>();
    m_instructionalText->Get<TextRenderable>()->Text = InstructionalText;
    m_instructionalText->Get<Transform>()->position = { 0, 1.5f, -5.f };
    m_instructionalText->Get<Transform>()->scale = float3{ 2.0f };

    m_controllers[Left].DebugText = m_entityStore->Create<TextDisplay>();
    m_controllers[Left].DebugText->Get<Transform>()->position = { -2.5, 1.25f, -4.f };
    m_controllers[Left].DebugText->Get<Transform>()->orientation = make_quaternion_from_axis_angle({ 0, 1, 0 }, DirectX::XM_PI * 0.15f);
    m_controllers[Left].DebugText->Get<Transform>()->scale = float3{ 2.0f };
    m_controllers[Left].DebugText->Get<TextRenderable>()->FontSize = 52.0f;

    m_controllers[Right].DebugText = m_entityStore->Create<TextDisplay>();
    m_controllers[Right].DebugText->Get<Transform>()->position = { 2.5, 1.25f, -4.f };
    m_controllers[Right].DebugText->Get<Transform>()->orientation = make_quaternion_from_axis_angle({ 0, 1, 0 }, -DirectX::XM_PI * 0.15f);
    m_controllers[Right].DebugText->Get<Transform>()->scale = float3{ 2.0f };
    m_controllers[Right].DebugText->Get<TextRenderable>()->FontSize = 52.0f;

    m_engine.Get<SpatialInteractionSystem>()->AddListener(shared_from_this());
}

void ToolboxSystem::Stop()
{
    m_engine.Get<SpatialInteractionSystem>()->RemoveListener(shared_from_this());
}

void ToolboxSystem::Update(float dt)
{
    static float fps[32] = {};
    static uint32_t currFps = 0;
    fps[currFps++] = dt;
    currFps %= _countof(fps);

    const float avgDt = std::accumulate(std::begin(fps), std::end(fps), 0.0f) / _countof(fps);

    m_instructionalText->Get<TextRenderable>()->Text = std::to_wstring(static_cast<int>(std::round(1.0f / avgDt))) + L" FPS\n\n" + InstructionalText.data();

    if (!m_showToolbox)
    {
        {
            int i = 0;
            for (auto& selectorPair : m_selectorObjects)
            {
                auto selector = selectorPair.second;

                const float offset = (i - floorf(m_selectorObjects.size() / 2.f)) / m_selectorObjects.size();
                selector->Get<Easing>()->TargetPosition = float3{ offset, 1.25f, -5.f };

                ++i;
            }
        }

        // Update the debug text for each Controller based on the currently selected tool
        for (size_t i = 0; i < m_controllers.size(); ++i)
        {
            std::wstring displayedText = std::wstring(ControllerHandToString(m_controllers[i].Hand)) + L": ";

            if (auto tool = m_controllers[i].Controller->Get<ToolComponent>())
            {
                displayedText += tool->title + L"\n\n" + tool->description;
            }

            m_controllers[i].DebugText->Get<TextRenderable>()->Text = displayedText;
        }
    }
    else
    {
        for (size_t i = 0; i < m_controllers.size(); ++i)
        {
            std::wstring displayedText = std::wstring(ControllerHandToString(m_controllers[i].Hand)) + L" switch to: ";

            const float3 controllerPosition = m_controllers[i].Controller->Get<Transform>()->position;

            for (auto[transform, selector] : m_engine.Get<EntityStore>()->GetComponents<Transform, ToolSelectorKey>())
            {
                if (HitTest(controllerPosition, transform->position, 0.15f))
                {
                    auto it = m_selectors.find(selector->type);
                    if (it != m_selectors.end())
                    {
                        displayedText += it->second->GetDisplayName();
                        displayedText += L"\n\n";
                        displayedText += it->second->GetInstructions();
                    }
                }
            }

            m_controllers[i].DebugText->Get<TextRenderable>()->Text = displayedText;
        }
    }
}


void ToolboxSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args)
{
    if (args.State().Source().Kind() != SpatialInteractionSourceKind::Controller)
        return;

    auto controller = FindController(args.State().Source());

    if (!controller)
        return;

    // Bring the toolbox in front of user
    if (args.PressKind() == SpatialInteractionPressKind::Menu)
    {
        m_showToolbox = !m_showToolbox;
        if (m_showToolbox)
        {
            auto holoScene = m_engine.Get<HolographicScene>();
            if (SpatialPointerPose pointerPose = SpatialPointerPose::TryGetAtTimestamp(holoScene->WorldCoordinateSystem(), holoScene->CurrentTimestamp()))
            {
                const float3 headPosition = pointerPose.Head().Position();
                const float3 forward = pointerPose.Head().ForwardDirection();
                const float3 headUp = pointerPose.Head().UpDirection();

                const float3 headDirection = normalize(float3{ forward.x, 0.0f, forward.z });

                float3 headRight = cross(headDirection, headUp);
                headRight.y = 0;
                headRight = normalize(headRight);

                const float3 toolkitCenter = headDirection * 0.5f;

                {
                    int i = 0;
                    for (auto& selectorPair : m_selectorObjects)
                    {
                        auto selector = selectorPair.second;

                        const float offset = (i - floorf(m_selectorObjects.size() / 2.f)) / m_selectorObjects.size();
                        const float3 targetPosition = toolkitCenter + headPosition + headRight * offset + float3{ 0, -0.3f, 0 };
                        selector->Get<Easing>()->TargetPosition = targetPosition;

                        ++i;
                    }
                }
            }
        }
    }
    else if (args.PressKind() == SpatialInteractionPressKind::Grasp && m_showToolbox)
    {
        if (const SpatialInteractionSourceLocation& location = controller->Get<MotionControllerComponent>()->location)
        {
            if (location.Position())
            {
                const float3 position = location.Position().Value();

                for (auto[transform, selector] : m_engine.Get<EntityStore>()->GetComponents<Transform, ToolSelectorKey>())
                {
                    if (HitTest(position, transform->position, 0.15f))
                    {
                        SwitchToolType(*controller, selector->type);
                        m_showToolbox = false;
                        break;
                    }
                }
            }
        }
    }
}

std::wstring_view ToolboxSystem::ControllerHandToString(ControllerHand hand) 
{
    return hand == Left ? L"Left" : L"Right";
}

SpatialInteractionSourceHandedness ToolboxSystem::ControllerHandToHandedness(ControllerHand hand) 
{
    return hand == Left ? SpatialInteractionSourceHandedness::Left : SpatialInteractionSourceHandedness::Right;
}

void ToolboxSystem::SwitchToolType(Entity& entity, const detail::type_id& newType)
{
    ToolComponent* tool = entity.Get<ToolComponent>();

    { // Disable old tool
        auto it = m_selectors.find(tool->toolType);
        if (it != m_selectors.end())
        {
            it->second->Deactivate(entity);
        }
    }

    { // Enable new tool
        auto it = m_selectors.find(newType);
        if (it != m_selectors.end())
        {
            it->second->Activate(entity);
            tool->toolType = it->second->type();
            tool->description = it->second->GetInstructions();
            tool->title = it->second->GetDisplayName();
        }
    }
}

SharedEntity ToolboxSystem::FindController(const SpatialInteractionSource& source)
{
    for (auto& context : m_controllers)
    {
        if (context.Controller->Get<MotionControllerComponent>()->IsSource(source))
        {
            return context.Controller;
        }
    }

    return nullptr;
}


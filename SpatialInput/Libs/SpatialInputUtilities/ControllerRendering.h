#pragma once

#include <memory>
#include <future>
#include <tuple>
#include <winrt\Windows.UI.Input.Spatial.h>

namespace Pbr {
    struct Model;
    struct Resources;
}

namespace ControllerRendering
{
    using ControllerModelKey = std::tuple<uint16_t, uint16_t, uint16_t, winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceHandedness>;

    // Values for articulating a controller, with defaults in the neutral state.
    struct ArticulateValues
    {
        float Grasp{ 0 };
        float Menu{ 0 };
        float Select{ 0 };
        float ThumbstickPress{ 0 };
        float ThumbstickX{ 0.5f };
        float ThumbstickY{ 0.5f };
        float TouchpadPress{ 0 };
        float TouchpadPressX{ 0.5f };
        float TouchpadPressY{ 0.5f };
        float TouchpadTouchX{ 0.5f };
        float TouchpadTouchY{ 0.5f };
        bool TouchIndicatorVisisble{ false };
    };

    ControllerModelKey GetControllerModelKey(winrt::Windows::UI::Input::Spatial::SpatialInteractionSource const& source);

    std::future<std::shared_ptr<Pbr::Model>> LoadRenderModel(
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSource const& source,
        _In_ ID3D11Device* device,
        _In_ ID3D11DeviceContext3* deviceContext,
        std::shared_ptr<Pbr::Resources> const& pbrResources);

    ArticulateValues GetArticulateValues(winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState const& sourceState);

    void ArticulateControllerModel(ArticulateValues const& articulateValues, Pbr::Model& controllerModel);
}
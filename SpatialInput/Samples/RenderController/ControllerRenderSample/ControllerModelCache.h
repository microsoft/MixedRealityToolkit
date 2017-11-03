#pragma once

#include <memory>

namespace Pbr {
    struct Model;
    struct Resources;
}

namespace DX {
    class DeviceResources;
}

struct ControllerModelCache
{
    ControllerModelCache(
        std::shared_ptr<DX::DeviceResources> deviceResources,
        std::shared_ptr<Pbr::Resources> pbrResources,
        winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& interactionManager);

    std::shared_ptr<Pbr::Model> TryGetControllerModel(winrt::Windows::UI::Input::Spatial::SpatialInteractionSource const& source);

    void CreateDeviceDependentResources();
    void ReleaseDeviceDependentResources();

private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};
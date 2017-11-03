#include "pch.h"
#include <robuffer.h>
#include <Pbr\GltfLoader.h>
#include <SpatialInputUtilities\ControllerRendering.h>
#include "Common\DeviceResources.h"
#include "ControllerModelCache.h"

using namespace std::placeholders;
using namespace DirectX;

using namespace Microsoft::WRL;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

namespace
{
    constexpr PCSTR TouchIndicatorNodeName = "TouchIndicator";
    constexpr PCSTR TouchIndicatorMaterialName = "TouchIndicator";
}

struct ControllerModelCache::Impl
{
    Impl(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<Pbr::Resources> pbrResources, winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& interactionManager)
        : m_deviceResources(deviceResources)
        , m_spatialInteractionManager(interactionManager)
        , m_pbrResources(pbrResources)
    {
        m_sourceDetectedToken = m_spatialInteractionManager.SourceDetected(std::bind(&Impl::OnSourceDetected, this, _1, _2));
    }

    ~Impl()
    {
        m_spatialInteractionManager.SourceDetected(m_sourceDetectedToken);
    }

    void OnSourceDetected(SpatialInteractionManager sender, SpatialInteractionSourceEventArgs args)
    {
        TryLoadControllerModel(args.State().Source());
    }

    std::future<void> TryLoadControllerModel(SpatialInteractionSource source)
    {
        const ControllerRendering::ControllerModelKey modelKey = ControllerRendering::GetControllerModelKey(source);
        if (GetRenderModel(modelKey))
        {
            co_return; // Model is already loaded.
        }

        const std::shared_ptr<Pbr::Model> controllerModel = co_await ControllerRendering::LoadRenderModel(source, m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), m_pbrResources);
        if (controllerModel)
        {
            auto lock = m_lock.LockExclusive();
            m_controllerMeshes[modelKey] = std::move(controllerModel);
        }
    }

    std::shared_ptr<Pbr::Model> GetRenderModel(const ControllerRendering::ControllerModelKey& modelKey)
    {
        auto readLock = m_lock.LockShared();
        auto controllerMeshIt = m_controllerMeshes.find(modelKey);
        if (controllerMeshIt == std::end(m_controllerMeshes))
        {
            return nullptr; // No model is cached.
        }

        return controllerMeshIt->second;
    }

    void Clear()
    {
        auto lock = m_lock.LockExclusive();
        m_controllerMeshes.clear();
    }

private:
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    Microsoft::WRL::Wrappers::SRWLock m_lock;
    std::map<ControllerRendering::ControllerModelKey, std::shared_ptr<Pbr::Model>> m_controllerMeshes;
    SpatialInteractionManager m_spatialInteractionManager;
    winrt::event_token m_sourceDetectedToken;
    std::shared_ptr<Pbr::Resources> m_pbrResources;
};

ControllerModelCache::ControllerModelCache(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<Pbr::Resources> pbrResources, winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& interactionManager)
    : m_impl(std::make_shared<Impl>(deviceResources, pbrResources, interactionManager))
{
}

std::shared_ptr<Pbr::Model> ControllerModelCache::TryGetControllerModel(SpatialInteractionSource const& source)
{
    const ControllerRendering::ControllerModelKey modelKey = ControllerRendering::GetControllerModelKey(source);
    return m_impl->GetRenderModel(modelKey);
}


void ControllerModelCache::CreateDeviceDependentResources()
{
}

void ControllerModelCache::ReleaseDeviceDependentResources()
{
    m_impl->Clear();
}
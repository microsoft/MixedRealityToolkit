////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "ControllerRenderer.h"
#include "Common\DirectXHelper.h"

using namespace ControllerRenderSample;
using namespace DirectX;
using namespace std::placeholders;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
ControllerRenderer::ControllerRenderer(
    std::shared_ptr<DX::DeviceResources> deviceResources,
    std::shared_ptr<Pbr::Resources> pbrResources,
    SpatialInteractionManager const& interactionManager) :
    m_deviceResources(std::move(deviceResources)),
    m_pbrResources(std::move(pbrResources)),
    m_interactionManager(interactionManager)
{
    // Bind a handler to the SourceDetected event. SourceDetected callbacks will automatically occur for existing sources.
    m_sourceDetectedEventToken = m_interactionManager.SourceDetected(bind(&ControllerRenderer::OnSourceDetected, this, _1, _2));
}

ControllerRenderer::~ControllerRenderer()
{
    m_interactionManager.SourceDetected(m_sourceDetectedEventToken);
}

void ControllerRenderer::Render(SpatialCoordinateSystem const& coordinateSystem, PerceptionTimestamp const& timestamp)
{
    // The timestamp should be from HolographicFramePrediction which is the predicted timestamp for when the
    // camera poses are predicted to be displayed. This timestamp is in the future which gives the most responsive
    // experience possible for rendering SpatialInteractionSources.
    for (const SpatialInteractionSourceState& sourceState : m_interactionManager.GetDetectedSourcesAtTimestamp(timestamp))
    {
        // Get the location of the SpatialInteractionSource for the earlier provided timestamp in the coordinate system provided.
        SpatialInteractionSourceLocation location = sourceState.Properties().TryGetLocation(coordinateSystem);
        if (!location || !location.Position() || !location.Orientation())
        {
            continue;
        }

        // Try to look up a renderable model for this SpatialInteractionSource.
        SpatialInteractionSource const& source = sourceState.Source();
        std::shared_ptr<Pbr::Model>& inputModel = m_spatialInputModels[source.Id()];
        if (inputModel)
        {
            // Articulate the controller model.
            // This is safe if a custom model such as a fallback is used since none of the documented nodes will exist.
            ControllerRendering::ArticulateControllerModel(ControllerRendering::GetArticulateValues(sourceState), *inputModel);

            // Set model materials to translucent if accuracy is low.
            const float alpha = location.PositionAccuracy() == SpatialInteractionSourcePositionAccuracy::High ? 1.0f : 0.33f;
            for (uint32_t i = 0; i < inputModel->GetPrimitiveCount(); i++)
            {
                std::shared_ptr<Pbr::Material>& primitiveMaterial = inputModel->GetPrimitive(i).GetMaterial();
                if (primitiveMaterial->Parameters.Get().BaseColorFactor.w != alpha)
                {
                    primitiveMaterial->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
                        data.BaseColorFactor.w = alpha;
                    });
                }
            }

            // Set transform on root node of controller to render it in the correct position and orientation.
            const XMMATRIX modelRotation = XMMatrixRotationQuaternion(XMLoadQuaternion(&location.Orientation().Value()));
            const XMMATRIX modelTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&location.Position().Value()));
            const XMMATRIX modelToWorld = modelRotation * modelTranslation;
            inputModel->GetNode(Pbr::RootNodeIndex).SetTransform(modelToWorld);

            // Render the controller.
            inputModel->Render(*m_pbrResources, m_deviceResources->GetD3DDeviceContext());
        }
    }
}

void ControllerRenderer::CreateDeviceDependentResources()
{
    // Reload the controller models.
    const PerceptionTimestamp now = PerceptionTimestampHelper::FromHistoricalTargetTime(winrt::clock::now());
    for (const auto& sourceState : m_interactionManager.GetDetectedSourcesAtTimestamp(now))
    {
        (void)CacheSpatialInputModelAsync(sourceState.Source());
    }
}

void ControllerRenderer::ReleaseDeviceDependentResources()
{
    m_spatialInputModels.clear();
    m_controllerModelCache.ReleaseDeviceDependentResources();
}

void ControllerRenderer::OnSourceDetected(SpatialInteractionManager const& sender, SpatialInteractionSourceEventArgs const& args)
{
    (void)CacheSpatialInputModelAsync(args.State().Source());
}

IAsyncAction ControllerRenderer::CacheSpatialInputModelAsync(SpatialInteractionSource const& source)
{
    constexpr char* FallbackModel = "__fallbackmodel__";

    std::shared_ptr<Pbr::Model>& model = m_spatialInputModels[source.Id()];

    // If a fallback model is loaded, try to load a model again. There is a chance the model
    // has been installed since the last attempt.
    if (model && model->Name != FallbackModel)
    {
        co_return; // The proper model is already cached.
    }

    std::shared_ptr<Pbr::Model> loadedModel;

    // std::future does not resume on the UI thread, so capture the context now and resume on it later.
    winrt::apartment_context originalContext;
    {
        // Try to get the controller model from the ControllerModelCache.
        if (std::shared_ptr<const Pbr::Model> controllerModel = co_await m_controllerModelCache.TryGetControllerModelAsync(m_pbrResources, source))
        {
            // Create a clone of the model returned by the cache so that it can be modified later.
            loadedModel = controllerModel->Clone(*m_pbrResources);
        }
    }
    co_await originalContext; // Resume on the original thread context.

    if (!loadedModel)
    {
        // No model available from the system. Instead, create and use a simple red cube fallback model.
        // Create 0.1 meter cube, and elongate it on the Z axis by 2x
        loadedModel = std::make_shared<Pbr::Model>();
        loadedModel->Name = FallbackModel;

        const Pbr::Node scaleNode = loadedModel->AddNode(XMMatrixScaling(1, 0.5, 2), Pbr::RootNodeIndex, "__");
        Pbr::Primitive cubePrimitive(*m_pbrResources, Pbr::PrimitiveBuilder().AddCube(0.1f, scaleNode.Index), Pbr::Material::CreateFlat(*m_pbrResources, Colors::Red, 0.3f));
        loadedModel->AddPrimitive(std::move(cubePrimitive));
    }

    model = loadedModel;
}

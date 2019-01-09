////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "SpinningCubeRenderer.h"
#include "Common\DirectXHelper.h"

using namespace ControllerRenderSample;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI::Input::Spatial;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SpinningCubeRenderer::SpinningCubeRenderer(
    std::shared_ptr<DX::DeviceResources> const& deviceResources,
    std::shared_ptr<Pbr::Resources> const& pbrResources) :
    m_deviceResources(deviceResources),
    m_pbrResources(pbrResources)
{
    CreateDeviceDependentResources();
}

// Called once per frame. Rotates the cube, and calculates and sets the model matrix
// relative to the position transform indicated by hologramPositionTransform.
void SpinningCubeRenderer::Update(DX::StepTimer const& timer)
{
    constexpr float degreesPerSecond = 45.f;
    constexpr float radiansPerSecond = XMConvertToRadians(degreesPerSecond);

    // Rotate the cube.
    const double yawRadiansDelta = timer.GetElapsedSeconds() * radiansPerSecond;
    m_orientation *= make_quaternion_from_yaw_pitch_roll((float)yawRadiansDelta, 0, 0);

    // Compute updated transform for model.
    const XMMATRIX modelRotation = XMMatrixRotationQuaternion(XMLoadQuaternion(&m_orientation));
    const XMMATRIX modelTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
    const XMMATRIX modelScale = XMMatrixScaling(0.25f, 0.25f, 0.25f);

    // Multiply to get the transform matrix.
    // Note that this transform does not enforce a particular coordinate system. The calling
    // class is responsible for rendering this content in a consistent manner.
    const XMMATRIX modelTransform = XMMatrixMultiply(modelScale, XMMatrixMultiply(modelRotation, modelTranslation));

    // The view and projection matrices are provided by the system; they are associated
    // with holographic cameras, and updated on a per-camera basis.
    // Here, we provide the model transform for the sample hologram.
    m_cubeModel->GetNode(Pbr::RootNodeIndex).SetTransform(modelTransform);
}

void SpinningCubeRenderer::Render()
{
    // Loading is asynchronous. Resources must be created before drawing can occur.
    if (!m_loadingComplete)
    {
        return;
    }

    m_cubeModel->Render(*m_pbrResources, m_deviceResources->GetD3DDeviceContext());
}

void SpinningCubeRenderer::CreateDeviceDependentResources()
{
    // Create a flat PBR material for the cube.
    std::shared_ptr<Pbr::Material> cubeMaterial = Pbr::Material::CreateFlat(
        *m_pbrResources,
        Colors::Gold /* base color */,
        0.1f /* roughness */,
        1.0f /* metallic */);

    // Create a cube primitive and load it into D3D buffers with associated material.
    Pbr::Primitive cubePrimitive(*m_pbrResources, Pbr::PrimitiveBuilder().AddCube(1.0f), std::move(cubeMaterial));

    // Create a model composed of the primitive.
    m_cubeModel = std::make_shared<Pbr::Model>();
    m_cubeModel->AddPrimitive(std::move(cubePrimitive));

    // The object is ready to be rendered.
    m_loadingComplete = true;
}

void SpinningCubeRenderer::ReleaseDeviceDependentResources()
{
    m_loadingComplete  = false;
    m_cubeModel.reset();
}


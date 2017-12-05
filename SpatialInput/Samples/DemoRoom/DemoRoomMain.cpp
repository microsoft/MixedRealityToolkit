////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "DemoRoomMain.h"

#include "AppLogicSystem.h"
#include "MotionControllerSystem.h"
#include "PaintStrokeSystem.h"
#include "ToolboxSystem.h"
#include "PaintingSystem.h"
#include "ShootingSystem.h"
#include "ThrowingSystem.h"

#include "EntityPrefabs.h"

using namespace concurrency;
using namespace DirectX;
using namespace std::placeholders;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

using namespace Neso;
using namespace DemoRoom;

// Loads and initializes application assets when the application is loaded.
DemoRoomMain::DemoRoomMain() :
    m_deviceResources(std::make_shared<DX::DeviceResources>())
{}

void DemoRoomMain::SetHolographicSpace(HolographicSpace const& holographicSpace)
{
    if (m_engine && m_engine->HasStarted())
    {
        m_engine->Stop();
        m_engine.reset();
    }

    if (holographicSpace == nullptr)
    {
        return;
    }

    m_deviceResources->SetHolographicSpace(holographicSpace);

    const auto pbrResources = std::make_shared<Pbr::Resources>(m_deviceResources->GetD3DDevice());

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseEnvironmentMap; 
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularEnvironmentMap;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brdlutTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyboxTexture;

    auto resourceLoadingTask = std::async(std::launch::async, [&] 
    {
        auto diffuseTextureFuture = DX::LoadDDSTextureAsync(m_deviceResources->GetD3DDevice(), L"ms-appx:///Media/Environment/DiffuseHDR.dds");
        auto specularTextureFuture = DX::LoadDDSTextureAsync(m_deviceResources->GetD3DDevice(), L"ms-appx:///Media/Environment/SpecularHDR.dds");
        auto skyboxTextureFuture = DX::LoadDDSTextureAsync(m_deviceResources->GetD3DDevice(), L"ms-appx:///Media/Environment/EnvHDR.dds");
        auto brdfLutFileDataFuture = DX::ReadDataAsync(L"ms-appx:///PBR/brdf_lut.png");

        diffuseEnvironmentMap = diffuseTextureFuture.get();
        specularEnvironmentMap = specularTextureFuture.get();
        skyboxTexture = skyboxTextureFuture.get();
        std::vector<byte> brdfLutFileData = brdfLutFileDataFuture.get();

        // Read the BRDF Lookup Table used by the PBR system into a DirectX texture.
        brdlutTexture = Pbr::Texture::LoadImage(m_deviceResources->GetD3DDevice(), brdfLutFileData.data(), static_cast<uint32_t>(brdfLutFileData.size()));
    });

    // Launch the loading tasks on another thread and wait for them to complete
    resourceLoadingTask.wait();

    pbrResources->SetBrdfLut(brdlutTexture.Get());
    pbrResources->SetEnvironmentMap(m_deviceResources->GetD3DDeviceContext(), specularEnvironmentMap.Get(), diffuseEnvironmentMap.Get());

    // System::Update is called in the order they were added to the Engine
    // Which is why we put the factories at the start, and the rendering at the end.
    m_engine = std::make_unique<Engine>();

    m_engine->Add<EntityStore>();
    m_engine->Add<ComponentStore>();
    m_engine->Add<HolographicScene>(holographicSpace);
    m_engine->Add<EasingSystem>();
    m_engine->Add<PhysicsSystem>();
    m_engine->Add<PbrModelCache>(pbrResources);

    m_engine->Add<SpatialInteractionSystem>();
    m_engine->Add<MotionControllerSystem>();
    m_engine->Add<AppLogicSystem>();

    m_engine->Add<ToolboxSystem>();
    m_engine->Add<ShootingInteractionSystem>();
    m_engine->Add<PaintingInteractionSystem>();
    m_engine->Add<ThrowingInteractionSystem>();

    m_engine->Add<PaintStrokeSystem>(pbrResources);

    m_engine->Add<HolographicRenderer>(m_deviceResources, pbrResources, skyboxTexture.Get());

    m_engine->Start();

    // Seed model cache
    auto pbrModelCache = m_engine->Get<PbrModelCache>();

    // Register a low poly sphere model.
    {
        Pbr::Primitive spherePrimitive(
            *pbrResources, 
            Pbr::PrimitiveBuilder().AddSphere(1.0f, 3), 
            Pbr::Material::CreateFlat(*pbrResources, Colors::White, 0.15f));

        // Add the primitive into a new model.
        auto sphereModel = std::make_shared<Pbr::Model>();
        sphereModel->AddPrimitive(std::move(spherePrimitive));
        pbrModelCache->RegisterModel(KnownModelNames::UnitSphere, std::move(sphereModel));
    }

    // Register a cube model.
    {
        // Load the primitive into D3D buffers with associated material
        Pbr::Primitive cubePrimitive(
            *pbrResources, 
            Pbr::PrimitiveBuilder().AddCube(1.0f), 
            Pbr::Material::CreateFlat(*pbrResources, Colors::White, 0.15f));

        // Add the primitive into a new model.
        auto cubeModel = std::make_shared<Pbr::Model>();
        cubeModel->AddPrimitive(std::move(cubePrimitive));
        pbrModelCache->RegisterModel(KnownModelNames::UnitCube, std::move(cubeModel));
    }

    // Register glb models.
    auto loadGLBModels = [this](
        std::wstring_view path,
        std::string_view name,
        std::optional<DirectX::XMFLOAT4X4> transform = std::nullopt,
        std::optional<DirectX::XMFLOAT4> color = std::nullopt) -> std::future<void>
    {
        auto pbrModelCache = m_engine->Get<PbrModelCache>();
        auto pbrResources = m_engine->Get<HolographicRenderer>()->GetPbrResources();

        std::vector<byte> fileData = co_await DX::ReadDataAsync(std::wstring(path));

        const DirectX::XMMATRIX modelTransform = transform.has_value()
            ? DirectX::XMLoadFloat4x4(&transform.value())
            : DirectX::XMMatrixIdentity();

        std::shared_ptr<Pbr::Model> pbrModel = Gltf::FromGltfBinary(
            *pbrResources,
            fileData.data(),
            (uint32_t)fileData.size(),
            modelTransform);

        if (color) {
            for (uint32_t i = 0; i < pbrModel->GetPrimitiveCount(); ++i) {
                pbrModel->GetPrimitive(i).GetMaterial()->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
                    data.BaseColorFactor = color.value();
                });
            }
        }

        debug_log("Loaded Model: %s", name.data());

        pbrModelCache->RegisterModel(name, std::move(pbrModel));
    };

    DirectX::XMFLOAT4X4 baseballScale;
    DirectX::XMStoreFloat4x4(&baseballScale, DirectX::XMMatrixScaling(0.15f, 0.15f, 0.15f));
    loadGLBModels(L"ms-appx:///Media/Models/Baseball.glb", KnownModelNames::Baseball, baseballScale, DirectX::XMFLOAT4{ 2.0f, 2.0f, 2.0f, 1.0f });

    DirectX::XMFLOAT4X4 gunScale;
    DirectX::XMStoreFloat4x4(&gunScale, DirectX::XMMatrixScaling(0.35f, 0.35f, 0.35f));
    loadGLBModels(L"ms-appx:///Media/Models/Gun.glb", KnownModelNames::Gun, gunScale);

    loadGLBModels(L"ms-appx:///Media/Models/PaintBrush.glb", KnownModelNames::PaintBrush);

    // We don't store the returned Floor Entity locally, so it lives foreeevvverrr
    m_engine->Get<EntityStore>()->Create<FloorPrefab>();

    // Reset timer on startup so the first update's delta time is sensible (albeit still small)
    m_timer.ResetElapsedTime();
}

DemoRoomMain::~DemoRoomMain()
{
    if (m_engine)
    {
        m_engine->Stop();
        m_engine.reset();
    }
}

// Updates the application state once per frame.
void DemoRoomMain::Update()
{
    m_timer.Tick([&]
    {
        m_engine->Update(static_cast<float>(m_timer.GetElapsedSeconds()));
    });
}

void DemoRoomMain::SaveAppState()
{
    m_deviceResources->Trim();

    if (m_engine)
    {
        m_engine->Suspend();
    }
}

void DemoRoomMain::LoadAppState()
{
    if (m_engine)
    {
        m_engine->Resume();
    }
}


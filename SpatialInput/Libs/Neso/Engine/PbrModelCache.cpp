////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrModelCache.h"
#include "CommonComponents.h" // For PbrRenderable
#include "EntityStore.h"

#include <Pbr\PbrModel.h>

namespace Neso {

    PbrModelCache::PbrModelCache(
        Engine& core,
        std::shared_ptr<Pbr::Resources> pbrResources) :
        System(core),
        m_pbrResources(std::move(pbrResources))
    {}

    void PbrModelCache::RegisterModel(std::string_view name, std::shared_ptr<Pbr::Model> model)
    {
        if (name.empty())
        {
            throw std::invalid_argument("Cannot register model with empty name");
        }

        m_modelMap[std::string(name)] = std::move(model);
    }

    PbrRenderable* PbrModelCache::SetModel(std::string_view name, PbrRenderable* pbrRenderableComponent)
    {
        pbrRenderableComponent->ModelName = name;

        auto it = m_modelMap.find(std::string(name));
        if (it != m_modelMap.end())
        {
            // Each instance gets its own copy of the model data (the heavyweight model data is immutable and reference counted).
            pbrRenderableComponent->Model = it->second->Clone(*m_pbrResources);
            debug_log("Model %s successfully attached to Entity [Deferred]", pbrRenderableComponent->ModelName.c_str());
        }

        return pbrRenderableComponent;
    }

    PbrRenderable* PbrModelCache::SetModel(std::string_view name, ComponentMap& componentMap)
    {
        PbrRenderable* pbrRenderableComponent = componentMap.Get<PbrRenderable>();
        return SetModel(name, pbrRenderableComponent);
    }

    bool PbrModelCache::ModelExists(std::string_view name)
    {
        return m_modelMap.find(std::string(name)) != m_modelMap.end();
    }

    void PbrModelCache::Update(float dt)
    {
        for (auto& componentSet : m_engine.Get<EntityStore>()->GetComponents<PbrRenderable>())
        {
            auto[pbrRenderable] = componentSet;

            // Find any PbrRenderable component which is waiting for a model to be loaded.
            if (!pbrRenderable->Model && pbrRenderable->ModelName.size() > 0)
            {
                (void)SetModel(pbrRenderable->ModelName.data(), pbrRenderable);
            }

            // Apply any material updates as needed.
            if (pbrRenderable->Model)
            {
                for (uint32_t i = 0; i < pbrRenderable->Model->GetPrimitiveCount(); ++i)
                {
                    const auto& material = pbrRenderable->Model->GetPrimitive(i).GetMaterial();

                    DirectX::XMFLOAT4 currentColor = material->Parameters.Get().BaseColorFactor;
                    if (pbrRenderable->Color && i == 0)
                    {
                        currentColor = DirectX::XMFLOAT4(*pbrRenderable->Color);
                    }

                    currentColor.w = (pbrRenderable->AlphaMultiplier) ? *pbrRenderable->AlphaMultiplier : 1.0f;

                    const DirectX::XMFLOAT4& materialBaseColor = material->Parameters.Get().BaseColorFactor;

                    if (currentColor.x != materialBaseColor.x ||
                        currentColor.y != materialBaseColor.y ||
                        currentColor.z != materialBaseColor.z ||
                        currentColor.w != materialBaseColor.w)
                    {
                        material->Parameters.Set([&](Pbr::Material::ConstantBufferData& data) {
                            data.BaseColorFactor = currentColor;
                        });
                    }
                }
            }
        }
    }

    void PbrModelCache::Uninitialize()
    {
        m_modelMap.clear();
    }

} // namespace Neso

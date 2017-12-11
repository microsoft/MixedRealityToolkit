////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Common\DeviceResources.h>

namespace Pbr { 
    struct Model;
    struct Resources;
}

namespace Neso {

    struct PbrRenderable;

    ////////////////////////////////////////////////////////////////////////////////
    // PbrModelCache
    // Stores all of the PbrModels in the system to avoid duplication. As well as
    // it allows for lazy-assignment of Model files to PbrRenderable components
    // This allows you to set the ModelName on a PbrRenderable component, and the 
    // PbrModelCache will automatically assign the Model field of the PbrRenderable once the model has been loaded
    class PbrModelCache : public System<PbrModelCache>
    {
    public:
        PbrModelCache(Engine& core, std::shared_ptr<Pbr::Resources> pbrResources);

        void RegisterModel(std::string_view name, std::shared_ptr<Pbr::Model> model);
        bool ModelExists(std::string_view name);
        PbrRenderable* SetModel(std::string_view name, PbrRenderable* pbrRenderableComponent);
        PbrRenderable* SetModel(std::string_view name, ComponentMap& componentMap);

    protected:
        void Update(float) override;
        void Uninitialize() override;

    private:
        std::shared_ptr<Pbr::Resources> m_pbrResources{ nullptr };
        std::map<std::string, std::shared_ptr<Pbr::Model>> m_modelMap;
    };
}

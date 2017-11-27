#pragma once

#include <Neso\Engine\Engine.h>
#include <Neso\Common\DeviceResources.h>

namespace Pbr { 
    struct Model;
    struct Resources;
}

namespace Neso {

    struct PbrRenderable;

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
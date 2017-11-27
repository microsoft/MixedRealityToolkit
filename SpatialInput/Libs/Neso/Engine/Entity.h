#pragma once

#include <Neso\Engine\EngineCommon.h>
#include <Neso\Engine\Component.h>

namespace Neso {
    
    class Engine;

    class EntityStore;
    class Entity;
    using SharedEntity = std::shared_ptr<Entity>;

    class Entity : public Destroyable, public Enableable, public std::enable_shared_from_this<Entity>
    {
    public:
        using EntityId = uint64_t;

        template<typename T>
        T* Get() {
            return m_components.Get<T>();
        }

        template<typename T>
        T* TryGet() {
            return m_components.TryGet<T>();
        }

        template<typename T>
        void Remove() {
            m_components.Remove<T>();
        }

        template<typename T>
        T* Add() {
            return m_components.Add<T>(m_engine.Get<ComponentStore>()->CreateComponent<T>());
        }

        Entity(ComponentMap components, EntityId id, Engine& core);
        virtual ~Entity();

        SharedEntity Clone() const;

        EntityId Id() const { return m_id; }

        Engine& GetEngine();
        const ComponentMap& GetComponents() const;
        void Destroy() override;
        void SetEnabled(bool) override;
        EntityId GetId() const;

    protected:
        ComponentMap m_components;
        EntityId m_id;
        Engine& m_engine;
    };

    template<typename... ComponentTs>
    struct EntityPrefab
    {
        static_assert(detail::all_components<ComponentTs...>::value, "All components should derive from Component");

        using Components = std::tuple<ComponentTs...>;

        static ComponentMap Make(ComponentStore& store)
        {
            return store.CreateComponentMap<ComponentTs...>();
        }
    };
}
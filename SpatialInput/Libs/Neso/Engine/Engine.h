////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <Neso\Engine\EngineCommon.h>
#include <Neso\Engine\EngineTypeTraits.h>
#include <Neso\Engine\Component.h>
#include <Neso\Engine\Entity.h>

namespace Neso 
{
    class Engine;

    ////////////////////////////////////////////////////////////////////////////////
    // SystemBase
    // Base abstract class for all Systems
    // A system operates on 1 or more components and performs function based on the data/state in each component
    class SystemBase abstract
    {
    public:
        SystemBase(Engine& engine);
        virtual ~SystemBase() = default;

        virtual detail::type_id type() const = 0;

    protected:
        friend Engine;

        virtual void Initialize() {}
        virtual void Start() {}
        virtual void Update(float /*dt*/) {}
        virtual void Stop() {}
        virtual void Uninitialize() {}

        Engine& m_engine;
    };


    // CRTP implementation helper 
    // Usage: class MySystem : System<MySystem> { /* functions + data members */ };
    template<typename T>
    class System : public SystemBase, public std::enable_shared_from_this<T>
    {
    public:
        using SystemBase::SystemBase;

        detail::type_id type() const override {
            return typeid(T);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Engine
    // The Engine manages all Systems and is reponsible for starting up, updating, and tearing down all systems
    // It is also the central location to query for other Systems or Add/Remove them from the Engine
    class Engine
    {
    public:
        template<typename SystemT>
        std::shared_ptr<SystemT> Get()
        {
            auto system = TryGet<SystemT>();
            fail_fast_if(system == nullptr);
            return system;
        }

        template<typename SystemT>
        std::shared_ptr<SystemT> TryGet()
        {
            static_assert(detail::is_system<SystemT>::value, "T should derive from System");

            fail_fast_if(!m_initialized);

            auto it = FindSystem(typeid(SystemT));
            return it == m_systems.end()
                ? nullptr
                : std::static_pointer_cast<SystemT>(*it);
        }

        template<typename SystemT, typename... Args>
        void Add(Args&&... args)
        {
            static_assert(detail::is_system<SystemT>::value, "T should derive from System");

            Add(typeid(SystemT), std::make_shared<SystemT>(*this, std::forward<Args>(args)...));
        }

        template<typename SystemT>
        void Remove()
        {
            static_assert(detail::is_system<SystemT>::value, "T should derive from System");

            fail_fast_if(!m_initialized || !m_started);

            Remove(typeid(SystemT));
        }

        Engine();
        virtual ~Engine();

        bool HasStarted() const;

        void Start();
        void Update(float dt);
        void Stop();
        void Suspend();
        void Resume();

    private:
        typedef std::vector<std::shared_ptr<SystemBase>> SystemCollection;
        SystemCollection m_systems;

        bool m_started = false;
        bool m_initialized = false;
        bool m_suspended = false;

        SystemCollection::iterator FindSystem(detail::type_id const& typeId);
        void Add(detail::type_id const& typeId, std::shared_ptr<SystemBase> system);
        void Remove(detail::type_id const& typeId);
    };

} // Neso


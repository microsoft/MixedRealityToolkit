////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "Engine.h"

using namespace Neso;

SystemBase::SystemBase(Engine& engine) : 
    m_engine(engine)
{}

void Engine::Start()
{
    fail_fast_if(m_initialized || m_started, "Shouldn't call Start if we already started");

    // Two-step initialization 
    // First Phase: (Initialize) : Systems can acquire local resources, but cannot access other systems
    // Second Phase: (Start) : Systems can access other systems and setup dependencies. 
    for (auto& system : m_systems) 
    {
        system->Initialize();
    }

    m_initialized = true;

    for (auto& system : m_systems)
    {
        system->Start();
    }

    m_started = true;
}

void Engine::Update(float dt)
{
    fail_fast_if(!m_started, "Shouldn't call Update if we haven't been started");

    if (m_suspended)
    {
        return;
    }

    for (auto& system : m_systems)
    {
        system->Update(dt);
    }
}

void Engine::Stop()
{
    fail_fast_if(!m_started, "Shouldn't call Stop if we haven't been started");

    m_started = false;

    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) 
    {
        (*it)->Stop();
    }

    m_initialized = false;

    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it)
    {
        (*it)->Uninitialize();
    }
}

void Engine::Suspend()
{
    m_suspended = true;
}

void Engine::Resume()
{
    m_suspended = false;
}

Engine::Engine() = default;

Engine::~Engine()
{
    fail_fast_if(m_initialized || m_started, "Engine should be in a clean state upon destruction");
}

bool Engine::HasStarted() const
{
    return m_started;
}

Engine::SystemCollection::iterator Engine::FindSystem(const detail::type_id& typeId)
{
    return std::find_if(m_systems.begin(), m_systems.end(),
        [typeId](const std::shared_ptr<SystemBase>& system)
    {
        return typeId == system->type();
    });
}

void Engine::Add(const detail::type_id& typeId, std::shared_ptr<SystemBase> system)
{
    fail_fast_if(m_started, "Invalid to add systems after the engine has started");

    auto it = FindSystem(typeId);
    fail_fast_if(it != m_systems.end());

    m_systems.push_back(std::move(system));
}

void Engine::Remove(const detail::type_id& typeId)
{
    fail_fast_if(m_started, "Invalid to remove systems after the engine has started");

    auto it = FindSystem(typeId);
    fail_fast_if(it == m_systems.end());

    m_systems.erase(it);
}


#include "pch.h"
#include "Entity.h"
#include "EntityStore.h"

using namespace Neso;

Entity::Entity(ComponentMap components, EntityId id, Engine& core) :
    m_components(std::move(components)),
    m_id(std::move(id)),
    m_engine(core)
{}

Entity::~Entity()
{
    Destroy();
}

SharedEntity Entity::Clone() const
{
    return m_engine.Get<EntityStore>()->CreateFromComponentMap(m_engine.Get<ComponentStore>()->Clone(m_components));
}

Engine& Entity::GetEngine()
{
    return m_engine;
}

const ComponentMap& Entity::GetComponents() const
{
    return m_components;
}

void Entity::Destroy()
{
    Destroyable::Destroy();

    for (auto& components : m_components)
    {
        components.second->Destroy();
    }
}

void Entity::SetEnabled(bool enable)
{
    Enableable::SetEnabled(enable);

    for (auto& components : m_components)
    {
        components.second->SetEnabled(enable);
    }
}

Entity::EntityId Entity::GetId() const
{
    return m_id;
}

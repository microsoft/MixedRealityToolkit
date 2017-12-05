////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "EntityStore.h"

using namespace Neso;

SharedEntity EntityStore::CreateFromComponentMap(ComponentMap components)
{
    return AddEntity(std::make_shared<Entity>(std::move(components), GetNextId(), m_engine));
}

void EntityStore::Update(float)
{
    Destroyable::PruneFromContainer(&m_objects);
}

SharedEntity EntityStore::AddEntity(SharedEntity obj)
{
    m_objects.push_back(obj);
    return obj;
}

Entity::EntityId EntityStore::GetNextId()
{
    return ++m_nextId;
}


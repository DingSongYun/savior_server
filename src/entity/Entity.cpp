#include "entity/Entity.hpp"
#include <algorithm>
#include <iostream>

namespace savior_server {
namespace entity {

// Entity类的实现
Entity::Entity(EntityId id) : m_id(id), m_active(true) {}

Entity::~Entity() = default;

EntityId Entity::GetId() const {
    return m_id;
}

void Entity::SetActive(bool active) {
    m_active = active;
}

bool Entity::IsActive() const {
    return m_active;
}

// EntityManager类的实现
EntityManager::EntityManager() : m_nextId(1) {}

EntityManager::~EntityManager() = default;

Entity* EntityManager::CreateEntity() {
    EntityId id = m_nextId++;
    auto entity = std::make_unique<Entity>(id);
    Entity* ptr = entity.get();
    
    m_entities[id] = std::move(entity);
    
    std::cout << "Created entity with ID: " << id << std::endl;
    return ptr;
}

void EntityManager::DestroyEntity(EntityId id) {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        std::cout << "Destroying entity with ID: " << id << std::endl;
        m_entities.erase(it);
    }
}

Entity* EntityManager::GetEntity(EntityId id) {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Entity*> EntityManager::GetAllEntities() {
    std::vector<Entity*> result;
    result.reserve(m_entities.size());
    
    for (const auto& pair : m_entities) {
        result.push_back(pair.second.get());
    }
    
    return result;
}

void EntityManager::Clear() {
    std::cout << "Clearing all entities (" << m_entities.size() << " entities)" << std::endl;
    m_entities.clear();
    m_nextId = 1;
}

size_t EntityManager::GetEntityCount() const {
    return m_entities.size();
}

} // namespace entity
} // namespace savior_server
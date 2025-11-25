#include "../../include/ECS/ComponentManager.h"
#include <algorithm>

namespace ECS {

// ========================================
// Entity Management
// ========================================

Entity ComponentManager::CreateEntity() {
    Entity entity = m_idGenerator.Create();
    m_entities.push_back(entity);
    return entity;
}

void ComponentManager::DestroyEntity(Entity entity) {
    // Remove all components for this entity
    RemoveTransform(entity);
    RemovePhysics(entity);
    RemoveRender(entity);
    RemoveCollider(entity);
    RemoveLight(entity);
    RemoveRotate(entity);
    RemoveOrbit(entity);
    RemovePlayerController(entity);
    
    // Remove from entity list
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
    
    // Recycle ID
    m_idGenerator.Destroy(entity);
}

// ========================================
// Transform Component
// ========================================

void ComponentManager::AddTransform(Entity entity, const TransformComponent& component) {
    // Check if entity already has this component
    if (m_entityToTransform.find(entity) != m_entityToTransform.end()) {
        // Update existing component
        size_t index = m_entityToTransform[entity];
        m_transforms[index] = component;
        return;
    }
    
    // Add new component
    size_t index = m_transforms.size();
    m_transforms.push_back(component);
    m_entityToTransform[entity] = index;
    m_transformToEntity[index] = entity;
}

void ComponentManager::RemoveTransform(Entity entity) {
    auto it = m_entityToTransform.find(entity);
    if (it == m_entityToTransform.end()) return;  // Component doesn't exist
    
    size_t index = it->second;
    size_t lastIndex = m_transforms.size() - 1;
    
    if (index != lastIndex) {
        // Swap with last element
        m_transforms[index] = m_transforms[lastIndex];
        
        // Update mappings for swapped element
        Entity swappedEntity = m_transformToEntity[lastIndex];
        m_entityToTransform[swappedEntity] = index;
        m_transformToEntity[index] = swappedEntity;
    }
    
    // Remove last element
    m_transforms.pop_back();
    m_entityToTransform.erase(entity);
    m_transformToEntity.erase(lastIndex);
}

TransformComponent* ComponentManager::GetTransform(Entity entity) {
    auto it = m_entityToTransform.find(entity);
    if (it == m_entityToTransform.end()) return nullptr;
    return &m_transforms[it->second];
}

bool ComponentManager::HasTransform(Entity entity) const {
    return m_entityToTransform.find(entity) != m_entityToTransform.end();
}

// ========================================
// Physics Component
// ========================================

void ComponentManager::AddPhysics(Entity entity, const PhysicsComponent& component) {
    if (m_entityToPhysics.find(entity) != m_entityToPhysics.end()) {
        size_t index = m_entityToPhysics[entity];
        m_physics[index] = component;
        return;
    }
    
    size_t index = m_physics.size();
    m_physics.push_back(component);
    m_entityToPhysics[entity] = index;
    m_physicsToEntity[index] = entity;
}

void ComponentManager::RemovePhysics(Entity entity) {
    auto it = m_entityToPhysics.find(entity);
    if (it == m_entityToPhysics.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_physics.size() - 1;
    
    if (index != lastIndex) {
        m_physics[index] = m_physics[lastIndex];
        Entity swappedEntity = m_physicsToEntity[lastIndex];
        m_entityToPhysics[swappedEntity] = index;
        m_physicsToEntity[index] = swappedEntity;
    }
    
    m_physics.pop_back();
    m_entityToPhysics.erase(entity);
    m_physicsToEntity.erase(lastIndex);
}

PhysicsComponent* ComponentManager::GetPhysics(Entity entity) {
    auto it = m_entityToPhysics.find(entity);
    if (it == m_entityToPhysics.end()) return nullptr;
    return &m_physics[it->second];
}

bool ComponentManager::HasPhysics(Entity entity) const {
    return m_entityToPhysics.find(entity) != m_entityToPhysics.end();
}

// ========================================
// Render Component
// ========================================

void ComponentManager::AddRender(Entity entity, const RenderComponent& component) {
    if (m_entityToRender.find(entity) != m_entityToRender.end()) {
        size_t index = m_entityToRender[entity];
        m_renders[index] = component;
        return;
    }
    
    size_t index = m_renders.size();
    m_renders.push_back(component);
    m_entityToRender[entity] = index;
    m_renderToEntity[index] = entity;
}

void ComponentManager::RemoveRender(Entity entity) {
    auto it = m_entityToRender.find(entity);
    if (it == m_entityToRender.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_renders.size() - 1;
    
    if (index != lastIndex) {
        m_renders[index] = m_renders[lastIndex];
        Entity swappedEntity = m_renderToEntity[lastIndex];
        m_entityToRender[swappedEntity] = index;
        m_renderToEntity[index] = swappedEntity;
    }
    
    m_renders.pop_back();
    m_entityToRender.erase(entity);
    m_renderToEntity.erase(lastIndex);
}

RenderComponent* ComponentManager::GetRender(Entity entity) {
    auto it = m_entityToRender.find(entity);
    if (it == m_entityToRender.end()) return nullptr;
    return &m_renders[it->second];
}

bool ComponentManager::HasRender(Entity entity) const {
    return m_entityToRender.find(entity) != m_entityToRender.end();
}

// ========================================
// Component Management - Collider
// ========================================
void ComponentManager::AddCollider(Entity entity, const ColliderComponent& component) {
    if (m_entityToCollider.find(entity) != m_entityToCollider.end()) {
        m_colliders[m_entityToCollider[entity]] = component;
        return;
    }
    size_t index = m_colliders.size();
    m_entityToCollider[entity] = index;
    m_colliderToEntity[index] = entity;
    m_colliders.push_back(component);
}

void ComponentManager::RemoveCollider(Entity entity) {
    if (m_entityToCollider.find(entity) == m_entityToCollider.end()) return;
    size_t index = m_entityToCollider[entity];
    size_t lastIndex = m_colliders.size() - 1;
    Entity lastEntity = m_colliderToEntity[lastIndex];
    m_colliders[index] = m_colliders[lastIndex];
    m_entityToCollider[lastEntity] = index;
    m_colliderToEntity[index] = lastEntity;
    m_entityToCollider.erase(entity);
    m_colliderToEntity.erase(lastIndex);
    m_colliders.pop_back();
}

ColliderComponent* ComponentManager::GetCollider(Entity entity) {
    if (m_entityToCollider.find(entity) == m_entityToCollider.end()) return nullptr;
    return &m_colliders[m_entityToCollider[entity]];
}

bool ComponentManager::HasCollider(Entity entity) const {
    return m_entityToCollider.find(entity) != m_entityToCollider.end();
}

// ========================================
// Component Management - Light
// ========================================
void ComponentManager::AddLight(Entity entity, const LightComponent& component) {
    if (m_entityToLight.find(entity) != m_entityToLight.end()) {
        m_lights[m_entityToLight[entity]] = component;
        return;
    }
    size_t index = m_lights.size();
    m_entityToLight[entity] = index;
    m_lightToEntity[index] = entity;
    m_lights.push_back(component);
}

void ComponentManager::RemoveLight(Entity entity) {
    if (m_entityToLight.find(entity) == m_entityToLight.end()) return;
    size_t index = m_entityToLight[entity];
    size_t lastIndex = m_lights.size() - 1;
    Entity lastEntity = m_lightToEntity[lastIndex];
    m_lights[index] = m_lights[lastIndex];
    m_entityToLight[lastEntity] = index;
    m_lightToEntity[index] = lastEntity;
    m_entityToLight.erase(entity);
    m_lightToEntity.erase(lastIndex);
    m_lights.pop_back();
}

LightComponent* ComponentManager::GetLight(Entity entity) {
    if (m_entityToLight.find(entity) == m_entityToLight.end()) return nullptr;
    return &m_lights[m_entityToLight[entity]];
}

bool ComponentManager::HasLight(Entity entity) const {
    return m_entityToLight.find(entity) != m_entityToLight.end();
}

// ========================================
// Component Management - Rotate
// ========================================
void ComponentManager::AddRotate(Entity entity, const RotateComponent& component) {
    if (m_entityToRotate.find(entity) != m_entityToRotate.end()) {
        m_rotates[m_entityToRotate[entity]] = component;
        return;
    }
    size_t index = m_rotates.size();
    m_entityToRotate[entity] = index;
    m_rotateToEntity[index] = entity;
    m_rotates.push_back(component);
}

void ComponentManager::RemoveRotate(Entity entity) {
    if (m_entityToRotate.find(entity) == m_entityToRotate.end()) return;
    size_t index = m_entityToRotate[entity];
    size_t lastIndex = m_rotates.size() - 1;
    Entity lastEntity = m_rotateToEntity[lastIndex];
    m_rotates[index] = m_rotates[lastIndex];
    m_entityToRotate[lastEntity] = index;
    m_rotateToEntity[index] = lastEntity;
    m_entityToRotate.erase(entity);
    m_rotateToEntity.erase(lastIndex);
    m_rotates.pop_back();
}

RotateComponent* ComponentManager::GetRotate(Entity entity) {
    if (m_entityToRotate.find(entity) == m_entityToRotate.end()) return nullptr;
    return &m_rotates[m_entityToRotate[entity]];
}

bool ComponentManager::HasRotate(Entity entity) const {
    return m_entityToRotate.find(entity) != m_entityToRotate.end();
}

// ========================================
// Component Management - Orbit
// ========================================
void ComponentManager::AddOrbit(Entity entity, const OrbitComponent& component) {
    if (m_entityToOrbit.find(entity) != m_entityToOrbit.end()) {
        m_orbits[m_entityToOrbit[entity]] = component;
        return;
    }
    size_t index = m_orbits.size();
    m_entityToOrbit[entity] = index;
    m_orbitToEntity[index] = entity;
    m_orbits.push_back(component);
}

void ComponentManager::RemoveOrbit(Entity entity) {
    if (m_entityToOrbit.find(entity) == m_entityToOrbit.end()) return;
    size_t index = m_entityToOrbit[entity];
    size_t lastIndex = m_orbits.size() - 1;
    Entity lastEntity = m_orbitToEntity[lastIndex];
    m_orbits[index] = m_orbits[lastIndex];
    m_entityToOrbit[lastEntity] = index;
    m_orbitToEntity[index] = lastEntity;
    m_entityToOrbit.erase(entity);
    m_orbitToEntity.erase(lastIndex);
    m_orbits.pop_back();
}

OrbitComponent* ComponentManager::GetOrbit(Entity entity) {
    if (m_entityToOrbit.find(entity) == m_entityToOrbit.end()) return nullptr;
    return &m_orbits[m_entityToOrbit[entity]];
}

bool ComponentManager::HasOrbit(Entity entity) const {
    return m_entityToOrbit.find(entity) != m_entityToOrbit.end();
}

// ========================================
// Queries
// ========================================
std::vector<Entity> ComponentManager::GetEntitiesWithTransform() const {
    std::vector<Entity> entities;
    entities.reserve(m_transforms.size());
    for (const auto& pair : m_transformToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithPhysics() const {
    std::vector<Entity> entities;
    entities.reserve(m_physics.size());
    for (const auto& pair : m_physicsToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRender() const {
    std::vector<Entity> entities;
    entities.reserve(m_renders.size());
    for (const auto& pair : m_renderToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithCollider() const {
    std::vector<Entity> entities;
    entities.reserve(m_colliders.size());
    for (const auto& pair : m_colliderToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithLight() const {
    std::vector<Entity> entities;
    entities.reserve(m_lights.size());
    for (const auto& pair : m_lightToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRotate() const {
    std::vector<Entity> entities;
    entities.reserve(m_rotates.size());
    for (const auto& pair : m_rotateToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithOrbit() const {
    std::vector<Entity> entities;
    entities.reserve(m_orbits.size());
    for (const auto& pair : m_orbitToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithPhysicsAndTransform() const {
    std::vector<Entity> entities;
    // Iterate smaller set for efficiency
    if (m_physics.size() < m_transforms.size()) {
        for (const auto& pair : m_physicsToEntity) {
            if (HasTransform(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    } else {
        for (const auto& pair : m_transformToEntity) {
            if (HasPhysics(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRenderAndTransform() const {
    std::vector<Entity> entities;
    if (m_renders.size() < m_transforms.size()) {
        for (const auto& pair : m_renderToEntity) {
            if (HasTransform(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    } else {
        for (const auto& pair : m_transformToEntity) {
            if (HasRender(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRotateAndTransform() const {
    std::vector<Entity> entities;
    if (m_rotates.size() < m_transforms.size()) {
        for (const auto& pair : m_rotateToEntity) {
            if (HasTransform(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    } else {
        for (const auto& pair : m_transformToEntity) {
            if (HasRotate(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithOrbitAndTransform() const {
    std::vector<Entity> entities;
    if (m_orbits.size() < m_transforms.size()) {
        for (const auto& pair : m_orbitToEntity) {
            if (HasTransform(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    } else {
        for (const auto& pair : m_transformToEntity) {
            if (HasOrbit(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    }
    return entities;
}

// ========================================
// PlayerController Component
// ========================================

void ComponentManager::AddPlayerController(Entity entity, const PlayerControllerComponent& component) {
    if (m_entityToPlayerController.find(entity) != m_entityToPlayerController.end()) {
        size_t index = m_entityToPlayerController[entity];
        m_playerControllers[index] = component;
        return;
    }
    
    size_t index = m_playerControllers.size();
    m_playerControllers.push_back(component);
    m_entityToPlayerController[entity] = index;
    m_playerControllerToEntity[index] = entity;
}

void ComponentManager::RemovePlayerController(Entity entity) {
    auto it = m_entityToPlayerController.find(entity);
    if (it == m_entityToPlayerController.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_playerControllers.size() - 1;
    
    if (index != lastIndex) {
        m_playerControllers[index] = m_playerControllers[lastIndex];
        Entity movedEntity = m_playerControllerToEntity[lastIndex];
        m_entityToPlayerController[movedEntity] = index;
        m_playerControllerToEntity[index] = movedEntity;
    }
    
    m_playerControllers.pop_back();
    m_entityToPlayerController.erase(entity);
    m_playerControllerToEntity.erase(lastIndex);
}

PlayerControllerComponent* ComponentManager::GetPlayerController(Entity entity) {
    auto it = m_entityToPlayerController.find(entity);
    if (it == m_entityToPlayerController.end()) return nullptr;
    return &m_playerControllers[it->second];
}

bool ComponentManager::HasPlayerController(Entity entity) const {
    return m_entityToPlayerController.find(entity) != m_entityToPlayerController.end();
}

std::vector<Entity> ComponentManager::GetEntitiesWithPlayerController() const {
    std::vector<Entity> entities;
    for (const auto& pair : m_playerControllerToEntity) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<Entity> ComponentManager::GetEntitiesWithPlayerControllerAndTransform() const {
    std::vector<Entity> entities;
    if (m_playerControllers.size() < m_transforms.size()) {
        for (const auto& pair : m_playerControllerToEntity) {
            if (HasTransform(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    } else {
        for (const auto& pair : m_transformToEntity) {
            if (HasPlayerController(pair.second)) {
                entities.push_back(pair.second);
            }
        }
    }
    return entities;
}

} // namespace ECS

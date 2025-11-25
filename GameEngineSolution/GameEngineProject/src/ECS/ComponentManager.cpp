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
    for (auto& [type, array] : m_componentArrays) {
        array->EntityDestroyed(entity);
    }
    
    // Remove from entity list
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
    
    // Recycle ID
    m_idGenerator.Destroy(entity);
}

} // namespace ECS

#pragma once

#include <cstdint>
#include <vector>

namespace ECS {

// Entity is just an ID
using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0;
constexpr Entity MAX_ENTITIES = 5000;

// Manages entity ID generation and recycling
class EntityIDGenerator {
public:
    EntityIDGenerator() : m_nextID(1) {}
    
    // Create a new entity ID
    Entity Create() {
        if (!m_freeList.empty()) {
            // Reuse recycled ID
            Entity id = m_freeList.back();
            m_freeList.pop_back();
            return id;
        }
        // Generate new ID
        return m_nextID++;
    }
    
    // Mark entity ID as available for reuse
    void Destroy(Entity entity) {
        if (entity != NULL_ENTITY) {
            m_freeList.push_back(entity);
        }
    }
    
    // Get total number of entities created (including recycled)
    uint32_t GetTotalCreated() const {
        return m_nextID - 1;
    }

    // Get number of currently active entities
    uint32_t GetActiveCount() const {
        return (m_nextID - 1) - static_cast<uint32_t>(m_freeList.size());
    }
    
private:
    Entity m_nextID;                    // Next ID to assign
    std::vector<Entity> m_freeList;     // Recycled IDs
};

} // namespace ECS

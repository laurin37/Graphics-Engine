#pragma once

#include "Entity.h"
#include "Components.h"
#include <unordered_map>
#include <vector>
#include <optional>
#include <typeindex>
#include <memory>
#include <stdexcept>
#include <cassert>

namespace ECS {

// ========================================
// IComponentArray
// Interface for generic component arrays
// ========================================
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

// ========================================
// ComponentArray<T>
// Generic sparse set storage for components
// ========================================
// ========================================
// ComponentArray<T>
// Generic sparse set storage for components
// ========================================
template<typename T>
class ComponentArray : public IComponentArray {
public:
    ComponentArray() {
        // Initialize sparse array with invalid index
        m_entityToIndex.resize(MAX_ENTITIES, 0xFFFFFFFF);
    }

    void InsertData(Entity entity, T component) {
        if (entity >= MAX_ENTITIES) {
            throw std::runtime_error("Entity ID out of range.");
        }

        if (m_entityToIndex[entity] != 0xFFFFFFFF) {
            // Component already exists, just update it
            m_componentArray[m_entityToIndex[entity]] = component;
            return;
        }

        // Add new component
        size_t newIndex = m_size;
        m_entityToIndex[entity] = newIndex;
        m_indexToEntity.push_back(entity);
        m_componentArray.push_back(component);
        m_size++;
    }

    void RemoveData(Entity entity) {
        if (entity >= MAX_ENTITIES || m_entityToIndex[entity] == 0xFFFFFFFF) {
            return; // Entity doesn't have this component
        }

        // Copy last element into deleted element's place to maintain density
        size_t indexOfRemovedEntity = m_entityToIndex[entity];
        size_t indexOfLastElement = m_size - 1;
        
        m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];

        // Update map to point to moved spot
        Entity entityOfLastElement = m_indexToEntity[indexOfLastElement];
        m_entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
        m_indexToEntity[indexOfRemovedEntity] = entityOfLastElement;

        m_entityToIndex[entity] = 0xFFFFFFFF;
        m_indexToEntity.pop_back();
        m_componentArray.pop_back();

        m_size--;
    }

    T& GetData(Entity entity) {
        if (entity >= MAX_ENTITIES || m_entityToIndex[entity] == 0xFFFFFFFF) {
            throw std::runtime_error("Retrieving non-existent component.");
        }
        return m_componentArray[m_entityToIndex[entity]];
    }

    bool HasData(Entity entity) const {
        return entity < MAX_ENTITIES && m_entityToIndex[entity] != 0xFFFFFFFF;
    }

    void EntityDestroyed(Entity entity) override {
        if (entity < MAX_ENTITIES && m_entityToIndex[entity] != 0xFFFFFFFF) {
            RemoveData(entity);
        }
    }

    // Direct access for systems
    std::vector<T>& GetComponentArray() {
        return m_componentArray;
    }
    
    // Helper to get entity for a specific index
    Entity GetEntityAtIndex(size_t index) {
        return m_indexToEntity[index];
    }

    size_t GetSize() const {
        return m_size;
    }

private:
    std::vector<T> m_componentArray;
    std::vector<size_t> m_entityToIndex; // Sparse array: Entity ID -> Index
    std::vector<Entity> m_indexToEntity; // Dense array: Index -> Entity ID
    size_t m_size = 0;
};

// ==================================================================================
// ComponentManager Class
// ----------------------------------------------------------------------------------
// The core of the ECS architecture.
// Responsible for:
// - Creating and destroying entities (IDs)
// - Managing component arrays (Sparse Sets) for each component type
// - Providing fast access to components for Systems
// - Ensuring memory locality for better cache performance
// ==================================================================================
class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;
    
    // ========================================
    // Entity Management
    // ========================================
    Entity CreateEntity() {
        Entity entity = m_idGenerator.Create();
        if (entity >= MAX_ENTITIES) {
            throw std::runtime_error("Maximum entity count exceeded.");
        }
        return entity;
    }

    void DestroyEntity(Entity entity) {
        m_idGenerator.Destroy(entity);
        // Notify all component arrays
        for (auto const& pair : m_componentArrays) {
            auto& componentArray = pair.second;
            if (componentArray) {
                componentArray->EntityDestroyed(entity);
            }
        }
    }

    size_t GetEntityCount() const { return m_idGenerator.GetActiveCount(); }

    // ========================================
    // Generic Component Management
    // ========================================
    
    template<typename T>
    void AddComponent(Entity entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    template<typename T>
    void RemoveComponent(Entity entity) {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        return GetComponentArray<T>()->GetData(entity);
    }
    
    template<typename T>
    T* GetComponentPtr(Entity entity) {
        auto array = GetComponentArray<T>();
        if (array->HasData(entity)) {
            return &array->GetData(entity);
        }
        return nullptr;
    }

    template<typename T>
    bool HasComponent(Entity entity) {
        return GetComponentArray<T>()->HasData(entity);
    }
    
    // Helper to get the raw array for systems
    template<typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray() {
        const char* typeName = typeid(T).name();

        if (m_componentArrays.find(typeName) == m_componentArrays.end()) {
            // Register component type if not exists
            m_componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[typeName]);
    }

private:
    // Entity ID generator
    EntityIDGenerator m_idGenerator;
    
    // Map from type name to a component array
    // Note: Using const char* from typeid(T).name() is generally stable for the lifetime of the program
    // but std::type_index is safer. However, for performance we might want a static ID.
    // For now, let's stick to map but optimize ComponentArray first.
    // Optimization: Use type name string directly as key (pointer comparison might not be safe across DLLs but fine here)
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> m_componentArrays;
};

} // namespace ECS

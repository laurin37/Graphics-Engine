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
template<typename T>
class ComponentArray : public IComponentArray {
public:
    void InsertData(Entity entity, T component) {
        if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
            // Component already exists, just update it
            m_componentArray[m_entityToIndex[entity]] = component;
            return;
        }

        // Add new component
        size_t newIndex = m_size;
        m_entityToIndex[entity] = newIndex;
        m_indexToEntity[newIndex] = entity;
        m_componentArray.push_back(component);
        m_size++;
    }

    void RemoveData(Entity entity) {
        if (m_entityToIndex.find(entity) == m_entityToIndex.end()) {
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

        m_entityToIndex.erase(entity);
        m_indexToEntity.erase(indexOfLastElement);
        m_componentArray.pop_back();

        m_size--;
    }

    T& GetData(Entity entity) {
        if (m_entityToIndex.find(entity) == m_entityToIndex.end()) {
            throw std::runtime_error("Retrieving non-existent component.");
        }
        return m_componentArray[m_entityToIndex[entity]];
    }

    bool HasData(Entity entity) const {
        return m_entityToIndex.find(entity) != m_entityToIndex.end();
    }

    void EntityDestroyed(Entity entity) override {
        if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
            RemoveData(entity);
        }
    }

    // Direct access for systems
    std::vector<T>& GetComponentArray() {
        return m_componentArray;
    }
    
    // Helper to get entity for a specific index (useful when iterating the vector directly)
    Entity GetEntityAtIndex(size_t index) {
        return m_indexToEntity[index];
    }

    size_t GetSize() const {
        return m_size;
    }

private:
    std::vector<T> m_componentArray;
    std::unordered_map<Entity, size_t> m_entityToIndex;
    std::unordered_map<size_t, Entity> m_indexToEntity;
    size_t m_size = 0;
};

// ========================================
// ComponentManager
// Central registry for all components
// ========================================
class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;
    
    // ========================================
    // Entity Management
    // ========================================
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    size_t GetEntityCount() const { return m_entities.size(); }

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
        std::type_index typeName = std::type_index(typeid(T));

        if (m_componentArrays.find(typeName) == m_componentArrays.end()) {
            // Register component type if not exists
            m_componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[typeName]);
    }
    
    // ========================================
    // Legacy Helpers (Optional - can be removed if we update all call sites)
    // ========================================
    Entity GetActiveCamera() {
        // This is a bit inefficient now, but acceptable for a single active camera lookup
        auto cameraArray = GetComponentArray<CameraComponent>();
        for (auto& cam : cameraArray->GetComponentArray()) {
            if (cam.isActive) {
                // We need to find the entity for this component. 
                // This is where the reverse map in ComponentArray is needed, but we don't have direct access to it from the value.
                // We have to iterate. Or we can just iterate the sparse set.
                // Let's iterate the sparse set via the array and use the index.
                // Wait, ComponentArray stores data densely.
                // We need to find the entity ID.
                // Let's add a helper to ComponentArray to get Entity from index.
                // Actually, for now, let's just return NULL_ENTITY if we can't easily find it, 
                // OR we can iterate all entities and check HasComponent<CameraComponent>.
                // Better: The CameraSystem should track the active camera.
                // For now, let's just leave this unimplemented or do a slow search if needed.
                // But wait, the original implementation had this.
                // Let's implement a slow search for now to maintain compatibility.
                // Actually, I added GetEntityAtIndex to ComponentArray.
                // But we can't easily map from `cam` reference to index without pointer arithmetic.
                // Let's just iterate the array by index.
                size_t index = &cam - &cameraArray->GetComponentArray()[0];
                return cameraArray->GetEntityAtIndex(index);
            }
        }
        return NULL_ENTITY;
    }

private:
    // Entity ID generator
    EntityIDGenerator m_idGenerator;
    
    // Set of all active entities (optional, mainly for DestroyEntity validation)
    // We can keep this to track valid entities.
    std::vector<Entity> m_entities;

    // Map from type string pointer to a component array
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_componentArrays;
};

} // namespace ECS

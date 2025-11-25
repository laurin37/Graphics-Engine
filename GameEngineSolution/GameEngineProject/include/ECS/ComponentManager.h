#pragma once

#include "Entity.h"
#include "Components.h"
#include <unordered_map>
#include <vector>
#include <optional>

namespace ECS {

// ========================================
// ComponentManager
// Central registry for all components
// Uses sparse set for O(1) access and cache-friendly iteration
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
    
    // ========================================
    // Component Management - Transform
    // ========================================
    void AddTransform(Entity entity, const TransformComponent& component);
    void RemoveTransform(Entity entity);
    TransformComponent* GetTransform(Entity entity);
    bool HasTransform(Entity entity) const;
    
    // ========================================
    // Component Management - Physics
    // ========================================
    void AddPhysics(Entity entity, const PhysicsComponent& component);
    void RemovePhysics(Entity entity);
    PhysicsComponent* GetPhysics(Entity entity);
    bool HasPhysics(Entity entity) const;
    
    // ========================================
    // Component Management - Render
    // ========================================
    void AddRender(Entity entity, const RenderComponent& component);
    void RemoveRender(Entity entity);
    RenderComponent* GetRender(Entity entity);
    bool HasRender(Entity entity) const;
    
    // ========================================
    // Component Management - Collider
    // ========================================
    void AddCollider(Entity entity, const ColliderComponent& component);
    void RemoveCollider(Entity entity);
    ColliderComponent* GetCollider(Entity entity);
    bool HasCollider(Entity entity) const;

    // ========================================
    // Component Management - Light
    // ========================================
    void AddLight(Entity entity, const LightComponent& component);
    void RemoveLight(Entity entity);
    LightComponent* GetLight(Entity entity);
    bool HasLight(Entity entity) const;

    // ========================================
    // Component Management - Rotate
    // ========================================
    void AddRotate(Entity entity, const RotateComponent& component);
    void RemoveRotate(Entity entity);
    RotateComponent* GetRotate(Entity entity);
    bool HasRotate(Entity entity) const;

    // ========================================
    // Component Management - Orbit
    // ========================================
    void AddOrbit(Entity entity, const OrbitComponent& component);
    void RemoveOrbit(Entity entity);
    OrbitComponent* GetOrbit(Entity entity);
    bool HasOrbit(Entity entity) const;
    
    // ========================================
    // Component Management - PlayerController
    // ========================================
    void AddPlayerController(Entity entity, const PlayerControllerComponent& component);
    void RemovePlayerController(Entity entity);
    PlayerControllerComponent* GetPlayerController(Entity entity);
    bool HasPlayerController(Entity entity) const;
    
    // ========================================
    // Queries (for systems to iterate entities)
    // ========================================
    std::vector<Entity> GetEntitiesWithTransform() const;
    std::vector<Entity> GetEntitiesWithPhysics() const;
    std::vector<Entity> GetEntitiesWithRender() const;
    std::vector<Entity> GetEntitiesWithCollider() const;
    std::vector<Entity> GetEntitiesWithLight() const;
    std::vector<Entity> GetEntitiesWithRotate() const;
    std::vector<Entity> GetEntitiesWithOrbit() const;
    std::vector<Entity> GetEntitiesWithPlayerController() const;
    
    // Get entities with multiple components
    std::vector<Entity> GetEntitiesWithPlayerControllerAndTransform() const;
    std::vector<Entity> GetEntitiesWithPhysicsAndTransform() const;
    std::vector<Entity> GetEntitiesWithRenderAndTransform() const;
    std::vector<Entity> GetEntitiesWithRotateAndTransform() const;
    std::vector<Entity> GetEntitiesWithOrbitAndTransform() const;
    
    // ========================================
    // Direct Component Array Access (for fast iteration)
    // ========================================
    std::vector<TransformComponent>& GetAllTransforms() { return m_transforms; }
    std::vector<PhysicsComponent>& GetAllPhysics() { return m_physics; }
    std::vector<RenderComponent>& GetAllRenders() { return m_renders; }
    std::vector<ColliderComponent>& GetAllColliders() { return m_colliders; }
    std::vector<LightComponent>& GetAllLights() { return m_lights; }
    std::vector<RotateComponent>& GetAllRotates() { return m_rotates; }
    std::vector<OrbitComponent>& GetAllOrbits() { return m_orbits; }
    std::vector<PlayerControllerComponent>& GetAllPlayerControllers() { return m_playerControllers; }
    
    // ========================================
    // Stats
    // ========================================
    size_t GetEntityCount() const { return m_entities.size(); }
    size_t GetTransformCount() const { return m_transforms.size(); }
    size_t GetPhysicsCount() const { return m_physics.size(); }
    size_t GetRenderCount() const { return m_renders.size(); }
    
private:
    // Entity ID generator
    EntityIDGenerator m_idGenerator;
    
    // Set of all active entities
    std::vector<Entity> m_entities;
    
    // ========================================
    // SPARSE SET STORAGE
    // Dense arrays: contiguous component data (cache-friendly iteration)
    // Sparse maps: entity -> index lookup (O(1) access)
    // Reverse maps: index -> entity for queries
    // ========================================
    
    // Transform components
    std::vector<TransformComponent> m_transforms;
    std::unordered_map<Entity, size_t> m_entityToTransform;
    std::unordered_map<size_t, Entity> m_transformToEntity;
    
    // Physics components
    std::vector<PhysicsComponent> m_physics;
    std::unordered_map<Entity, size_t> m_entityToPhysics;
    std::unordered_map<size_t, Entity> m_physicsToEntity;
    
    // Render components
    std::vector<RenderComponent> m_renders;
    std::unordered_map<Entity, size_t> m_entityToRender;
    std::unordered_map<size_t, Entity> m_renderToEntity;
    
    // Collider components
    std::vector<ColliderComponent> m_colliders;
    std::unordered_map<Entity, size_t> m_entityToCollider;
    std::unordered_map<size_t, Entity> m_colliderToEntity;

    // Light components
    std::vector<LightComponent> m_lights;
    std::unordered_map<Entity, size_t> m_entityToLight;
    std::unordered_map<size_t, Entity> m_lightToEntity;

    // Rotate components
    std::vector<RotateComponent> m_rotates;
    std::unordered_map<Entity, size_t> m_entityToRotate;
    std::unordered_map<size_t, Entity> m_rotateToEntity;

    // Orbit components
    std::vector<OrbitComponent> m_orbits;
    std::unordered_map<Entity, size_t> m_entityToOrbit;
    std::unordered_map<size_t, Entity> m_orbitToEntity;
    
    // PlayerController components
    std::vector<PlayerControllerComponent> m_playerControllers;
    std::unordered_map<Entity, size_t> m_entityToPlayerController;
    std::unordered_map<size_t, Entity> m_playerControllerToEntity;
};

} // namespace ECS

#include "../../../include/ECS/Systems/ECSRenderSystem.h"
#include "../../../include/Renderer/Mesh.h"
#include "../../../include/Renderer/Material.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace ECS {

void RenderSystem::Render(Renderer* renderer, Camera& camera) {
    if (!renderer) return;
    
    // Iterate over all render components
    auto renderArray = m_componentManager.GetComponentArray<RenderComponent>();
    auto& renderVec = renderArray->GetComponentArray();
    
    for (size_t i = 0; i < renderVec.size(); ++i) {
        Entity entity = renderArray->GetEntityAtIndex(i);
        RenderComponent& render = renderVec[i];
        
        if (!render.mesh || !render.material) continue;
        
        if (!m_componentManager.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Build world matrix from transform
        XMMATRIX scaleMatrix = XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z);
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
        XMMATRIX translationMatrix = XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
        
        XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
        
        // Set world matrix (renderer should have method for this)
        // Note: Since we don't have direct access to renderer internals,
        // we'll need to work with the existing GameObject rendering system temporarily
        
        // TODO: This is a placeholder - proper integration requires updating Renderer
        // to accept raw world matrices instead of GameObject pointers
        
        // For now, we can't directly render ECS entities without refactoring Renderer
        // This demonstrates the ECS architecture, but full integration needs more work
    }
}

void RenderSystem::RenderDebug(Renderer* renderer, Camera& camera) {
    if (!renderer) return;
    
    // Get all entities with Collider and Transform
    std::vector<AABB> aabbs;
    
    auto colliderArray = m_componentManager.GetComponentArray<ColliderComponent>();
    auto& colliderVec = colliderArray->GetComponentArray();
    
    for (size_t i = 0; i < colliderVec.size(); ++i) {
        Entity entity = colliderArray->GetEntityAtIndex(i);
        ColliderComponent& collider = colliderVec[i];
        
        if (!collider.enabled) continue;
        
        if (!m_componentManager.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Calculate world-space AABB
        AABB worldAABB;
        worldAABB.extents.x = transform.scale.x * collider.localAABB.extents.x;
        worldAABB.extents.y = transform.scale.y * collider.localAABB.extents.y;
        worldAABB.extents.z = transform.scale.z * collider.localAABB.extents.z;
        
        worldAABB.center.x = transform.position.x + (transform.scale.x * collider.localAABB.center.x);
        worldAABB.center.y = transform.position.y + (transform.scale.y * collider.localAABB.center.y);
        worldAABB.center.z = transform.position.z + (transform.scale.z * collider.localAABB.center.z);
        
        aabbs.push_back(worldAABB);
    }
    
    renderer->RenderDebugAABBs(camera, aabbs);
}

} // namespace ECS

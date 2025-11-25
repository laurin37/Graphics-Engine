#include "../../../include/ECS/Systems/CameraSystem.h"
#include "../../../include/ECS/Components.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace ECS {

void CameraSystem::Update(ComponentManager& cm) {
    // Get all entities with cameras
    std::vector<Entity> cameras = cm.GetEntitiesWithCamera();
    
    for (Entity entity : cameras) {
        CameraComponent* camera = cm.GetCamera(entity);
        TransformComponent* transform = cm.GetTransform(entity);
        
        if (!camera || !transform) continue;
        
        // Update projection matrix
        float fovRadians = XMConvertToRadians(camera->fov);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovRadians, camera->aspectRatio, 
                                                   camera->nearPlane, camera->farPlane);
        XMStoreFloat4x4(&camera->projectionMatrix, proj);
        
        // Update view matrix from transform
        XMVECTOR pos = XMLoadFloat3(&transform->position);
        XMVECTOR rot = XMLoadFloat3(&transform->rotation);
        
        // Apply camera offset
        XMVECTOR offset = XMLoadFloat3(&camera->positionOffset);
        pos = XMVectorAdd(pos, offset);
        
        // Calculate forward, right, up vectors from rotation
        XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(rot.m128_f32[0], rot.m128_f32[1], rot.m128_f32[2]);
        XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
        XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);
        
        // Look at target = position + forward
        XMVECTOR target = XMVectorAdd(pos, forward);
        
        // Create view matrix
        XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&camera->viewMatrix, view);
    }
}

bool CameraSystem::GetActiveCamera(ComponentManager& cm, XMMATRIX& viewOut, XMMATRIX& projOut) {
    Entity activeCameraEntity = cm.GetActiveCamera();
    
    // Check if we have a valid entity (NULL_ENTITY = 0)
    if (activeCameraEntity == NULL_ENTITY) {
        return false; // No active camera
    }
    
    CameraComponent* camera = cm.GetCamera(activeCameraEntity);
    if (!camera) return false;
    
    // Load matrices
    viewOut = XMLoadFloat4x4(&camera->viewMatrix);
    projOut = XMLoadFloat4x4(&camera->projectionMatrix);
    
    return true;
}

} // namespace ECS

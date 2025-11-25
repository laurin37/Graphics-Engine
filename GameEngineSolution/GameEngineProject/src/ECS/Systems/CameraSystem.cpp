#include "../../../include/ECS/Systems/CameraSystem.h"
#include "../../../include/ECS/Components.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace ECS {

void CameraSystem::Update(ComponentManager& cm) {
    // Iterate over all camera components
    auto cameraArray = cm.GetComponentArray<CameraComponent>();
    auto& cameraVec = cameraArray->GetComponentArray();
    
    for (size_t i = 0; i < cameraVec.size(); ++i) {
        Entity entity = cameraArray->GetEntityAtIndex(i);
        CameraComponent& camera = cameraVec[i];
        
        if (!cm.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = cm.GetComponent<TransformComponent>(entity);
        
        // Optional player controller for pitch
        PlayerControllerComponent* controller = cm.GetComponentPtr<PlayerControllerComponent>(entity);
        
        // Update projection matrix
        float fovRadians = XMConvertToRadians(camera.fov);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovRadians, camera.aspectRatio, 
                                                   camera.nearPlane, camera.farPlane);
        XMStoreFloat4x4(&camera.projectionMatrix, proj);
        
        // Update view matrix from transform
        XMVECTOR pos = XMLoadFloat3(&transform.position);
        float pitch = controller ? controller->viewPitch : transform.rotation.x;
        float yaw = transform.rotation.y;
        float roll = transform.rotation.z;
        XMVECTOR rot = XMVectorSet(pitch, yaw, roll, 0.0f);
        
        // Apply camera offset
        XMVECTOR offset = XMLoadFloat3(&camera.positionOffset);
        pos = XMVectorAdd(pos, offset);
        
        // Calculate forward, right, up vectors from rotation
        XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(rot.m128_f32[0], rot.m128_f32[1], rot.m128_f32[2]);
        XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotMatrix);
        XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotMatrix);
        
        // Look at target = position + forward
        XMVECTOR target = XMVectorAdd(pos, forward);
        
        // Create view matrix
        XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
        XMStoreFloat4x4(&camera.viewMatrix, view);
    }
}

bool CameraSystem::GetActiveCamera(ComponentManager& cm, XMMATRIX& viewOut, XMMATRIX& projOut) {
    Entity activeCameraEntity = cm.GetActiveCamera();
    
    // Check if we have a valid entity (NULL_ENTITY = 0)
    if (activeCameraEntity == NULL_ENTITY) {
        return false; // No active camera
    }
    
    if (!cm.HasComponent<CameraComponent>(activeCameraEntity)) return false;
    CameraComponent& camera = cm.GetComponent<CameraComponent>(activeCameraEntity);
    
    // Load matrices
    viewOut = XMLoadFloat4x4(&camera.viewMatrix);
    projOut = XMLoadFloat4x4(&camera.projectionMatrix);
    
    return true;
}

} // namespace ECS

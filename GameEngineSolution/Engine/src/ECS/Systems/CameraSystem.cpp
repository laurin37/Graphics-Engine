#include "../../../include/ECS/Systems/CameraSystem.h"
#include "../../../include/ECS/Components.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace ECS {

void CameraSystem::Update(float deltaTime) {
    // Iterate over all camera components
    auto cameraArray = m_componentManager.GetComponentArray<CameraComponent>();
    auto& cameraVec = cameraArray->GetComponentArray();
    
    for (size_t i = 0; i < cameraVec.size(); ++i) {
        Entity entity = cameraArray->GetEntityAtIndex(i);
        CameraComponent& camera = cameraVec[i];
        
        if (!m_componentManager.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Optional player controller for pitch
        PlayerControllerComponent* controller = m_componentManager.GetComponentPtr<PlayerControllerComponent>(entity);
        
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

bool CameraSystem::GetActiveCamera(DirectX::XMMATRIX& viewOut, DirectX::XMMATRIX& projOut) {
    Entity activeCameraEntity = GetActiveCameraEntity();
    
    // Check if we have a valid entity (NULL_ENTITY = 0)
    if (activeCameraEntity == NULL_ENTITY) {
        return false; // No active camera
    }
    
    if (!m_componentManager.HasComponent<CameraComponent>(activeCameraEntity)) return false;
    CameraComponent& camera = m_componentManager.GetComponent<CameraComponent>(activeCameraEntity);
    
    // Load matrices
    viewOut = XMLoadFloat4x4(&camera.viewMatrix);
    projOut = XMLoadFloat4x4(&camera.projectionMatrix);
    
    return true;
}

Entity CameraSystem::GetActiveCameraEntity() {
    auto cameraArray = m_componentManager.GetComponentArray<CameraComponent>();
    // Iterate to find active camera
    // Note: This is O(N) where N is number of cameras (usually small)
    for (size_t i = 0; i < cameraArray->GetSize(); ++i) {
        Entity entity = cameraArray->GetEntityAtIndex(i);
        CameraComponent& cam = cameraArray->GetData(entity);
        if (cam.isActive) {
            return entity;
        }
    }
    return NULL_ENTITY;
}

} // namespace ECS

#include "../../include/UI/DebugUIRenderer.h"
#include "../../include/UI/UIRenderer.h"
#include <cmath>
#include <string>

DebugUIRenderer::DebugUIRenderer() : m_enabled(true)
{
}

std::vector<DebugUIRenderer::DebugMessage> DebugUIRenderer::s_messages;

void DebugUIRenderer::Render(
    UIRenderer* uiRenderer,
    const SimpleFont& font,
    int fps,
    bool bloomEnabled,
    bool debugCollisionEnabled,
    ECS::ComponentManager& componentManager,
    ECS::Entity activeCamera
)
{
    if (!m_enabled || !uiRenderer) return;

    // Color definitions
    float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    float cyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
    float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float orange[4] = { 1.0f, 0.5f, 0.0f, 1.0f };

    float yPos = 10.0f;
    const float lineHeight = 30.0f;

    // Draw FPS
    std::string fpsString = "FPS: " + std::to_string(fps);
    uiRenderer->DrawString(font, fpsString, 10.0f, yPos, 24.0f, green);
    yPos += lineHeight;

    // Draw Bloom status
    std::string bloomStatus = bloomEnabled ? "[B] Bloom: ON" : "[B] Bloom: OFF";
    uiRenderer->DrawString(font, bloomStatus, 10.0f, yPos, 24.0f, yellow);
    yPos += lineHeight;

    // Draw Debug Collision status
    std::string debugStatus = debugCollisionEnabled ? "[H] Debug: ON" : "[H] Debug: OFF";
    uiRenderer->DrawString(font, debugStatus, 10.0f, yPos, 24.0f, cyan);
    yPos += lineHeight;

    // Draw Debug UI toggle hint
    std::string debugUIHint = "[F1] Debug UI: ON";
    uiRenderer->DrawString(font, debugUIHint, 10.0f, yPos, 24.0f, orange);
    yPos += lineHeight;

    // Draw entity count
    size_t entityCount = componentManager.GetEntityCount();
    std::string entityInfo = "ECS Entities: " + std::to_string(entityCount);
    uiRenderer->DrawString(font, entityInfo, 10.0f, yPos, 20.0f, white);
    yPos += lineHeight;

    // Show player information if exists
    auto playerArray = componentManager.GetComponentArray<ECS::PlayerControllerComponent>();
    if (playerArray && playerArray->GetSize() > 0) {
        ECS::Entity playerEntity = playerArray->GetEntityAtIndex(0);
        
        ECS::TransformComponent* playerTrans = componentManager.GetComponentPtr<ECS::TransformComponent>(playerEntity);
        ECS::PhysicsComponent* playerPhys = componentManager.GetComponentPtr<ECS::PhysicsComponent>(playerEntity);
        ECS::ColliderComponent* playerCol = componentManager.GetComponentPtr<ECS::ColliderComponent>(playerEntity);

        if (playerTrans && playerCol) {
            float feetY = playerTrans->position.y - playerTrans->scale.y;
            float headY = playerTrans->position.y + playerTrans->scale.y;

            const float scaleY = std::abs(playerTrans->scale.y);
            const float colliderCenterOffsetY = playerCol->localAABB.center.y * scaleY;
            const float colliderHalfHeight = playerCol->localAABB.extents.y * scaleY;

            float feetYAABB = playerTrans->position.y + colliderCenterOffsetY - colliderHalfHeight;
            float headYAABB = playerTrans->position.y + colliderCenterOffsetY + colliderHalfHeight;

            // Feet Position
            char playerPosBuffer[128];
            snprintf(playerPosBuffer, sizeof(playerPosBuffer), "Player Feet: (%.2f, %.2f, %.2f)",
                playerTrans->position.x, feetY, playerTrans->position.z);
            uiRenderer->DrawString(font, playerPosBuffer, 10.0f, yPos, 20.0f, green);
            yPos += lineHeight;

            // Feet AABB
            char playerPosAABBBuffer[128];
            snprintf(playerPosAABBBuffer, sizeof(playerPosAABBBuffer), "Player Feet AABB: (%.2f, %.2f, %.2f)",
                playerTrans->position.x, feetYAABB, playerTrans->position.z);
            uiRenderer->DrawString(font, playerPosAABBBuffer, 10.0f, yPos, 20.0f, green);
            yPos += lineHeight;

            // Head Position
            char headPosBuffer[128];
            snprintf(headPosBuffer, sizeof(headPosBuffer), "Player Head: (%.2f, %.2f, %.2f)",
                playerTrans->position.x, headY, playerTrans->position.z);
            uiRenderer->DrawString(font, headPosBuffer, 10.0f, yPos, 20.0f, green);
            yPos += lineHeight;

            // Head AABB
            char headPosAABBBuffer[128];
            snprintf(headPosAABBBuffer, sizeof(headPosAABBBuffer), "Player Head AABB: (%.2f, %.2f, %.2f)",
                playerTrans->position.x, headYAABB, playerTrans->position.z);
            uiRenderer->DrawString(font, headPosAABBBuffer, 10.0f, yPos, 20.0f, green);
            yPos += lineHeight;
        }

        if (playerPhys) {
            char playerVelBuffer[128];
            snprintf(playerVelBuffer, sizeof(playerVelBuffer), "Velocity: (%.2f, %.2f, %.2f) Grounded: %s",
                playerPhys->velocity.x, playerPhys->velocity.y, playerPhys->velocity.z,
                playerPhys->isGrounded ? "YES" : "NO");
            uiRenderer->DrawString(font, playerVelBuffer, 10.0f, yPos, 18.0f, yellow);
            yPos += lineHeight;
        }

        // Show Ammo
        ECS::WeaponComponent* playerWeapon = componentManager.GetComponentPtr<ECS::WeaponComponent>(playerEntity);
        if (playerWeapon) {
            char ammoBuffer[128];
            snprintf(ammoBuffer, sizeof(ammoBuffer), "Ammo: %d / %d", playerWeapon->currentAmmo, playerWeapon->maxAmmo);
            uiRenderer->DrawString(font, ammoBuffer, 10.0f, yPos, 24.0f, yellow);
            yPos += lineHeight;

            char grenadeBuffer[128];
            snprintf(grenadeBuffer, sizeof(grenadeBuffer), "Grenades: %d / %d", playerWeapon->projectileAmmo, playerWeapon->maxProjectileAmmo);
            uiRenderer->DrawString(font, grenadeBuffer, 10.0f, yPos, 24.0f, yellow);
            yPos += lineHeight;
        }
    }

    // Show active camera position
    if (activeCamera != ECS::NULL_ENTITY) {
        ECS::TransformComponent* camTrans = componentManager.GetComponentPtr<ECS::TransformComponent>(activeCamera);
        ECS::CameraComponent* camComp = componentManager.GetComponentPtr<ECS::CameraComponent>(activeCamera);
        
        if (camTrans && camComp) {
            // Calculate actual camera position (Transform + Offset)
            float camX = camTrans->position.x + camComp->positionOffset.x;
            float camY = camTrans->position.y + camComp->positionOffset.y;
            float camZ = camTrans->position.z + camComp->positionOffset.z;

            char camPosBuffer[128];
            snprintf(camPosBuffer, sizeof(camPosBuffer), "Camera Pos: (%.2f, %.2f, %.2f)", camX, camY, camZ);
            uiRenderer->DrawString(font, camPosBuffer, 10.0f, yPos, 20.0f, white);
            yPos += lineHeight;
        }
    }

    // Show Health of all entities
    auto healthArray = componentManager.GetComponentArray<ECS::HealthComponent>();
    if (healthArray && healthArray->GetSize() > 0) {
        yPos += lineHeight; // Add some spacing
        uiRenderer->DrawString(font, "--- Health Status ---", 10.0f, yPos, 20.0f, white);
        yPos += lineHeight;

        for (size_t i = 0; i < healthArray->GetSize(); ++i) {
            ECS::Entity entity = healthArray->GetEntityAtIndex(i);
            ECS::HealthComponent& health = healthArray->GetData(entity);

            char healthBuffer[128];
            snprintf(healthBuffer, sizeof(healthBuffer), "Entity %d: %.1f / %.1f %s", 
                entity, health.currentHealth, health.maxHealth, health.isDead ? "(DEAD)" : "");
            
            // Color code based on health
            float* color = green;
            if (health.isDead) color = white; // Greyed out/White for dead
            else if (health.currentHealth < health.maxHealth * 0.3f) color = orange; // Low health

            uiRenderer->DrawString(font, healthBuffer, 10.0f, yPos, 18.0f, color);
            yPos += lineHeight;
        }
    }

    // Render Debug Messages
    if (!s_messages.empty()) {
        yPos += lineHeight;
        uiRenderer->DrawString(font, "--- Debug Log ---", 10.0f, yPos, 20.0f, white);
        yPos += lineHeight;

        for (const auto& msg : s_messages) {
            float* color = white;
            if (msg.timeRemaining < 1.0f) color = yellow; // Fade out effect (simulated)
            
            uiRenderer->DrawString(font, msg.text, 10.0f, yPos, 24.0f, color);
            yPos += lineHeight;
        }
    }
}

void DebugUIRenderer::Update(float deltaTime) {
    for (auto it = s_messages.begin(); it != s_messages.end(); ) {
        it->timeRemaining -= deltaTime;
        if (it->timeRemaining <= 0.0f) {
            it = s_messages.erase(it);
        } else {
            ++it;
        }
    }
}

void DebugUIRenderer::AddMessage(const std::string& message, float duration) {
    s_messages.push_back({ message, duration, duration });
    // Keep only last 10 messages
    if (s_messages.size() > 10) {
        s_messages.erase(s_messages.begin());
    }
}

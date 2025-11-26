#include "Systems/HealthSystem.h"
#include <iostream>
#include <format>

void HealthSystem::Update(float deltaTime) {
    auto healthArray = m_componentManager.GetComponentArray<ECS::HealthComponent>();
    
    // Iterate backwards to safely remove entities
    for (int i = (int)healthArray->GetSize() - 1; i >= 0; --i) {
        ECS::Entity entity = healthArray->GetEntityAtIndex(i);
        ECS::HealthComponent& health = healthArray->GetData(entity);

        if (health.isDead) continue;

        // Regeneration
        if (health.regenerationRate > 0.0f && health.currentHealth < health.maxHealth) {
            health.currentHealth += health.regenerationRate * deltaTime;
            if (health.currentHealth > health.maxHealth) {
                health.currentHealth = health.maxHealth;
            }
        }

        // Death check
        if (health.currentHealth <= 0.0f) {
            health.currentHealth = 0.0f;
            health.isDead = true;
            m_componentManager.DestroyEntity(entity);
        }
    }
}

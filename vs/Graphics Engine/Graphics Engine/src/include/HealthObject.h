#pragma once

#include "EnginePCH.h"
#include <DirectXMath.h>
#include "GameObject.h"
#include "Material.h" // To modify the object's color
#include "Mesh.h"     // Include actual Mesh header

class HealthObject : public GameObject
{
public:
    HealthObject();
    HealthObject(float maxHealth, const DirectX::XMFLOAT3& initialPosition);
    ~HealthObject();

    void TakeDamage(float amount);
    float GetCurrentHealth() const { return m_currentHealth; }
    float GetMaxHealth() const { return m_maxHealth; }
    bool IsDead() const { return m_currentHealth <= 0; }

private:
    void UpdateColor();

    float m_maxHealth;
    float m_currentHealth;
};

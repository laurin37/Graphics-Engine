#pragma once

#include <DirectXMath.h>

#include "../Utils/EnginePCH.h"
#include "GameObject.h"
#include "../Renderer/Material.h"
#include "../Renderer/Mesh.h"

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

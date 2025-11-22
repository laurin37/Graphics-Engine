#include "include/HealthObject.h"

using namespace DirectX;

HealthObject::HealthObject()
    : GameObject(nullptr, std::shared_ptr<Material>()), // Pass default values to base constructor
      m_maxHealth(100.0f),
      m_currentHealth(100.0f)
{
    SetName(L"HealthObject"); // Use SetName()
    UpdateColor();
}

HealthObject::HealthObject(float maxHealth, const XMFLOAT3& initialPosition)
    : GameObject(nullptr, std::shared_ptr<Material>()), // Pass default values to base constructor
      m_maxHealth(maxHealth),
      m_currentHealth(maxHealth)
{
    SetName(L"HealthObject"); // Use SetName()
    m_transform.SetPosition(initialPosition.x, initialPosition.y, initialPosition.z);
    UpdateColor();
}

HealthObject::~HealthObject()
{
}

void HealthObject::TakeDamage(float amount)
{
    m_currentHealth -= amount;
    if (m_currentHealth < 0)
    {
        m_currentHealth = 0;
    }
    UpdateColor();

    // TODO: Add logic for when health reaches 0 (e.g., destroy object)
}

void HealthObject::UpdateColor()
{
    // Interpolate color from green (1,0,0) to red (0,1,0) based on health percentage
    // Health = 100% -> Green (0,1,0)
    // Health = 0%   -> Red   (1,0,0)
    float healthRatio = m_currentHealth / m_maxHealth; // 1.0 = full health, 0.0 = no health

    // Linear interpolation for R and G channels
    // R: from 0 (green) to 1 (red)
    // G: from 1 (green) to 0 (red)
    XMFLOAT4 color;
    color.x = 1.0f - healthRatio; // Red component
    color.y = healthRatio;       // Green component
    color.z = 0.0f;              // Blue component
    color.w = 1.0f;              // Alpha component

    if (m_material) // Use inherited m_material directly
    {
        m_material->SetDiffuseColor(color);
    }
}

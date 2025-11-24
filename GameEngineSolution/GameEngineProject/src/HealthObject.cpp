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
    // Interpolate color from green (0,1,0) to red (1,0,0) based on health percentage
    float healthRatio = m_currentHealth / m_maxHealth; // 1.0 = full health, 0.0 = no health

    // Clamp ratio
    if (healthRatio < 0.0f) healthRatio = 0.0f;
    if (healthRatio > 1.0f) healthRatio = 1.0f;

    // Linear interpolation
    // R: 0 -> 1 (as health decreases) => 1.0 - healthRatio
    // G: 1 -> 0 (as health decreases) => healthRatio
    // B: 0
    XMFLOAT4 color;
    color.x = 1.0f - healthRatio; // Red increases as health drops
    color.y = healthRatio;       // Green decreases as health drops
    color.z = 0.0f;
    color.w = 1.0f;

    if (m_material)
    {
        m_material->SetDiffuseColor(color);
    }
}

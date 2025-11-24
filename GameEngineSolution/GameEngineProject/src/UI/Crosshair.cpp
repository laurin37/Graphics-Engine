#include "../../include/UI/Crosshair.h"

Crosshair::Crosshair()
    : m_size(30.0f)
{
    // White color
    m_color[0] = 1.0f;
    m_color[1] = 1.0f;
    m_color[2] = 1.0f;
    m_color[3] = 1.0f;
}

void Crosshair::Draw(UIRenderer* uiRenderer, const SimpleFont& font, float screenWidth, float screenHeight)
{
    if (!uiRenderer) return;

    // Calculate center position
    // Note: DrawString coordinates are usually top-left. 
    // We want to center the '+' character.
    // Assuming the character width/height is roughly equal to m_size.
    
    float x = (screenWidth / 2.0f) - (m_size / 2.0f);
    float y = (screenHeight / 2.0f) - (m_size / 2.0f);

    uiRenderer->DrawString(font, "+", x, y, m_size, m_color);
}

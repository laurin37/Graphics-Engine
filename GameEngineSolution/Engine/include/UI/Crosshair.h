#pragma once

#include "UIRenderer.h"
#include "SimpleFont.h"

class Crosshair
{
public:
    Crosshair();
    ~Crosshair() = default;

    void Draw(UIRenderer* uiRenderer, const SimpleFont& font, float screenWidth, float screenHeight);

private:
    float m_size;
    float m_color[4];
};

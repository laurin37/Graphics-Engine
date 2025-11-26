#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

#include "UIRenderer.h"

struct Glyph
{
    float u, v;         // Top-left UV
    float u2, v2;       // Bottom-right UV
    float width;        // Width in pixels
    float height;       // Height in pixels
    float xOffset;      // Horizontal offset
    float yOffset;      // Vertical offset
    float xAdvance;     // Horizontal advance
};

class SimpleFont
{
public:
    SimpleFont();
    ~SimpleFont() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture, const std::vector<Glyph>& glyphs = {});
    
    std::vector<SpriteVertex> GenerateVerticesForString(const std::string& text, float x, float y, float size, const float color[4]) const;

    ID3D11ShaderResourceView* GetTexture() const;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_fontTexture;
    std::vector<Glyph> m_glyphs;
    bool m_isMonospace = true; // Fallback for legacy 16x16 grid
};
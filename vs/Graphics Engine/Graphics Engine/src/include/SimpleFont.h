#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include "UIRenderer.h" // For SpriteVertex

class SimpleFont
{
public:
    SimpleFont();
    ~SimpleFont() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture);
    
    std::vector<SpriteVertex> GenerateVerticesForString(const std::string& text, float x, float y, float size, const float color[4]) const;

    ID3D11ShaderResourceView* GetTexture() const;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_fontTexture;
};
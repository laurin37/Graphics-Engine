#include "include/SimpleFont.h"

SimpleFont::SimpleFont() {}

void SimpleFont::Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture)
{
    m_fontTexture = fontTexture;
}

ID3D11ShaderResourceView* SimpleFont::GetTexture() const
{
    return m_fontTexture.Get();
}

std::vector<SpriteVertex> SimpleFont::GenerateVerticesForString(const std::string& text, float x, float y, float size, const float color[4]) const
{
    std::vector<SpriteVertex> vertices;
    vertices.reserve(text.length() * 6);

    float curX = x;
    float curY = y;

    const float charStep = size * 0.6f;
    const float uvStep = 1.0f / 16.0f;

    for (char c : text)
    {
        if (c == '\n')
        {
            curX = x;
            curY += size;
            continue;
        }

        unsigned char uc = static_cast<unsigned char>(c);
        int col = uc % 16;
        int row = uc / 16;

        float u = col * uvStep;
        float v = row * uvStep;

        SpriteVertex v0, v1, v2, v3;

        // Top-Left
        v0.pos = { curX, curY, 0.0f };
        v0.uv = { u, v };
        v0.color = { color[0], color[1], color[2], color[3] };

        // Top-Right
        v1.pos = { curX + size, curY, 0.0f };
        v1.uv = { u + uvStep, v };
        v1.color = { color[0], color[1], color[2], color[3] };

        // Bottom-Left
        v2.pos = { curX, curY + size, 0.0f };
        v2.uv = { u, v + uvStep };
        v2.color = { color[0], color[1], color[2], color[3] };

        // Bottom-Right
        v3.pos = { curX + size, curY + size, 0.0f };
        v3.uv = { u + uvStep, v + uvStep };
        v3.color = { color[0], color[1], color[2], color[3] };
        
        // Triangle 1
        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);

        // Triangle 2
        vertices.push_back(v2);
        vertices.push_back(v1);
        vertices.push_back(v3);

        curX += charStep;
    }

    return vertices;
}
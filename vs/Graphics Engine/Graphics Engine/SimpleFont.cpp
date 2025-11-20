#include "SimpleFont.h"

SimpleFont::SimpleFont() {}

void SimpleFont::Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture)
{
    m_fontTexture = fontTexture;
}

void SimpleFont::DrawString(Graphics& gfx, const std::string& text, float x, float y, float size, float color[4])
{
    // Removed early return to allow drawing debugging quads if texture is missing
    // if (!m_fontTexture) return; 

    int numChars = 0;
    float curX = x;
    float curY = y;

    const float charStep = size * 0.6f;
    const float uvStep = 1.0f / 16.0f;

    for (char c : text)
    {
        if (numChars >= MAX_CHARS) break;
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

        // Top-Left
        m_spriteBuffer[numChars * 6 + 0].pos = { curX, curY, 0.0f };
        m_spriteBuffer[numChars * 6 + 0].uv = { u, v };
        m_spriteBuffer[numChars * 6 + 0].color = { color[0], color[1], color[2], color[3] };

        // Top-Right
        m_spriteBuffer[numChars * 6 + 1].pos = { curX + size, curY, 0.0f };
        m_spriteBuffer[numChars * 6 + 1].uv = { u + uvStep, v };
        m_spriteBuffer[numChars * 6 + 1].color = { color[0], color[1], color[2], color[3] };

        // Bottom-Left
        m_spriteBuffer[numChars * 6 + 2].pos = { curX, curY + size, 0.0f };
        m_spriteBuffer[numChars * 6 + 2].uv = { u, v + uvStep };
        m_spriteBuffer[numChars * 6 + 2].color = { color[0], color[1], color[2], color[3] };

        // Bottom-Left
        m_spriteBuffer[numChars * 6 + 3].pos = { curX, curY + size, 0.0f };
        m_spriteBuffer[numChars * 6 + 3].uv = { u, v + uvStep };
        m_spriteBuffer[numChars * 6 + 3].color = { color[0], color[1], color[2], color[3] };

        // Top-Right
        m_spriteBuffer[numChars * 6 + 4].pos = { curX + size, curY, 0.0f };
        m_spriteBuffer[numChars * 6 + 4].uv = { u + uvStep, v };
        m_spriteBuffer[numChars * 6 + 4].color = { color[0], color[1], color[2], color[3] };

        // Bottom-Right
        m_spriteBuffer[numChars * 6 + 5].pos = { curX + size, curY + size, 0.0f };
        m_spriteBuffer[numChars * 6 + 5].uv = { u + uvStep, v + uvStep };
        m_spriteBuffer[numChars * 6 + 5].color = { color[0], color[1], color[2], color[3] };

        curX += charStep;
        numChars++;
    }

    gfx.DrawUI(m_spriteBuffer, numChars * 6, m_fontTexture.Get());
}
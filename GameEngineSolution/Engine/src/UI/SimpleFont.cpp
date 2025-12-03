#include "../../include/UI/SimpleFont.h"

SimpleFont::SimpleFont() {}

void SimpleFont::Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture, const std::vector<Glyph>& glyphs)
{
    m_fontTexture = fontTexture;
    m_glyphs = glyphs;
    m_isMonospace = m_glyphs.empty();
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

    // Legacy monospace fallback
    const float monoCharStep = size * 0.6f;
    const float monoUvStep = 1.0f / 16.0f;

    for (char c : text)
    {
        if (c == '\n')
        {
            curX = x;
            curY += size;
            continue;
        }

        unsigned char uc = static_cast<unsigned char>(c);
        
        float u, v, u2, v2;
        float w, h, xOff, yOff, advance;

        if (m_isMonospace)
        {
            // Legacy 16x16 grid logic
            int col = uc % 16;
            int row = uc / 16;
            u = col * monoUvStep;
            v = row * monoUvStep;
            u2 = u + monoUvStep;
            v2 = v + monoUvStep;
            w = size;
            h = size;
            xOff = 0;
            yOff = 0;
            advance = monoCharStep;
        }
        else
        {
            // Variable width logic
            if (uc >= m_glyphs.size()) uc = '?'; // Safety fallback
            if (uc >= m_glyphs.size()) continue;

            const Glyph& g = m_glyphs[uc];
            u = g.u;
            v = g.v;
            u2 = g.u2;
            v2 = g.v2;
            
            // Scale glyph dimensions to target font size
            // Assuming glyphs were baked at a reference size (e.g. 64px)
            float scale = size / 64.0f; 
            
            w = g.width * scale;
            h = g.height * scale;
            xOff = g.xOffset * scale;
            yOff = g.yOffset * scale;
            advance = g.xAdvance * scale;
        }

        SpriteVertex sv0, sv1, sv2, sv3;

        // Top-Left
        sv0.pos = { curX + xOff, curY + yOff, 0.0f };
        sv0.uv = { u, v };
        sv0.color = { color[0], color[1], color[2], color[3] };

        // Top-Right
        sv1.pos = { curX + xOff + w, curY + yOff, 0.0f };
        sv1.uv = { u2, v };
        sv1.color = { color[0], color[1], color[2], color[3] };

        // Bottom-Left
        sv2.pos = { curX + xOff, curY + yOff + h, 0.0f };
        sv2.uv = { u, v2 };
        sv2.color = { color[0], color[1], color[2], color[3] };

        // Bottom-Right
        sv3.pos = { curX + xOff + w, curY + yOff + h, 0.0f };
        sv3.uv = { u2, v2 };
        sv3.color = { color[0], color[1], color[2], color[3] };
        
        // Triangle 1
        vertices.push_back(sv0);
        vertices.push_back(sv1);
        vertices.push_back(sv2);

        // Triangle 2
        vertices.push_back(sv2);
        vertices.push_back(sv1);
        vertices.push_back(sv3);

        curX += advance;
    }

    return vertices;
}

DirectX::XMFLOAT2 SimpleFont::MeasureString(const std::string& text, float size) const
{
    float width = 0.0f;
    float height = size; // Base height

    // Legacy monospace fallback
    const float monoCharStep = size * 0.6f;

    for (char c : text)
    {
        if (c == '\n')
        {
            // Multiline support not fully implemented in measure yet, just return max width of single line for now
            continue; 
        }

        unsigned char uc = static_cast<unsigned char>(c);
        float advance;

        if (m_isMonospace)
        {
            advance = monoCharStep;
        }
        else
        {
            if (uc >= m_glyphs.size()) uc = '?';
            if (uc >= m_glyphs.size()) continue;

            const Glyph& g = m_glyphs[uc];
            float scale = size / 64.0f;
            advance = g.xAdvance * scale;
        }

        width += advance;
    }

    return { width, height };
}
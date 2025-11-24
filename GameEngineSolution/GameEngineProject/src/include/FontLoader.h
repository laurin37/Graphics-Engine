#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include "SimpleFont.h" // For Glyph struct

struct FontData
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    std::vector<Glyph> glyphs;
};

class FontLoader
{
public:
    // Loads a TTF file, rasterizes it to a texture, and returns the texture + glyph data.
    // fontFaceName: The internal name of the font (e.g. "Arial", "Minecraft"). 
    //               If empty, it tries to use the filename as a guess.
    static FontData Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filePath, const std::wstring& fontFaceName, float fontSize = 64.0f);
};

#include "include/FontLoader.h"
#include "include/Logger.h"
#define NOMINMAX
#include <windows.h>
#include <stdexcept>
#include <algorithm>
#include <cmath>

FontData FontLoader::Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filePath, const std::wstring& fontFaceName, float fontSize)
{
    // 1. Load the font file
    if (AddFontResourceEx(filePath.c_str(), FR_PRIVATE, 0) == 0)
    {
        LOG_ERROR("Failed to load font file: " + std::string(filePath.begin(), filePath.end()));
        throw std::runtime_error("Failed to load font file");
    }

    // 2. Create a memory DC
    HDC hdc = CreateCompatibleDC(NULL);
    if (!hdc)
    {
        RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);
        throw std::runtime_error("Failed to create compatible DC");
    }

    // 3. Create the font
    // We use a negative height to match character height in pixels
    HFONT hFont = CreateFont(
        -static_cast<int>(fontSize), 
        0, 0, 0, 
        FW_NORMAL, 
        FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, 
        ANTIALIASED_QUALITY, 
        DEFAULT_PITCH | FF_DONTCARE, 
        fontFaceName.c_str());

    if (!hFont)
    {
        DeleteDC(hdc);
        RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);
        throw std::runtime_error("Failed to create GDI font");
    }

    SelectObject(hdc, hFont);

    // 4. Setup texture atlas
    const int textureWidth = 512;
    const int textureHeight = 512;
    
    // Create a DIB section (bitmap) that we can write to
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = textureWidth;
    bmi.bmiHeader.biHeight = -textureHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // BGRA
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hBitmap)
    {
        DeleteObject(hFont);
        DeleteDC(hdc);
        RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);
        throw std::runtime_error("Failed to create DIB section");
    }

    SelectObject(hdc, hBitmap);

    // Clear bitmap (transparent black)
    memset(pBits, 0, textureWidth * textureHeight * 4);

    // Set text properties
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);

    // 5. Render characters
    std::vector<Glyph> glyphs(256); // Support ASCII + Extended
    int curX = 0;
    int curY = 0;
    int rowHeight = 0;

    // Padding between characters
    const int padding = 2;

    for (int i = 32; i < 127; ++i) // ASCII printable
    {
        wchar_t c = static_cast<wchar_t>(i);
        
        // Get character size
        SIZE size;
        GetTextExtentPoint32(hdc, &c, 1, &size);

        // Get ABC widths for precise spacing
        ABC abc;
        GetCharABCWidths(hdc, i, i, &abc);

        if (curX + size.cx + padding > textureWidth)
        {
            curX = 0;
            curY += rowHeight + padding;
            rowHeight = 0;
        }

        if (curY + size.cy > textureHeight)
        {
            LOG_WARNING("Font texture atlas full! Some characters may be missing.");
            break;
        }

        // Draw text
        TextOut(hdc, curX, curY, &c, 1);

        // Store glyph info
        Glyph& g = glyphs[i];
        g.u = static_cast<float>(curX) / textureWidth;
        g.v = static_cast<float>(curY) / textureHeight;
        g.width = static_cast<float>(size.cx);
        g.height = static_cast<float>(size.cy);
        g.u2 = g.u + (g.width / textureWidth);
        g.v2 = g.v + (g.height / textureHeight);
        
        // GDI specific offsets
        g.xOffset = static_cast<float>(abc.abcA); 
        g.yOffset = 0; // GDI draws from top-left, so usually 0
        g.xAdvance = static_cast<float>(abc.abcA + abc.abcB + abc.abcC);

        curX += size.cx + padding;
        rowHeight = (std::max)(rowHeight, static_cast<int>(size.cy));
    }

    // 6. Convert to D3D11 Texture
    
    uint8_t* pixels = static_cast<uint8_t*>(pBits);
    std::vector<uint8_t> textureData(textureWidth * textureHeight * 4);

    for (int i = 0; i < textureWidth * textureHeight; ++i)
    {
        // GDI draws white text on black background.
        // Pixel layout is B G R A (little endian 32-bit)
        uint8_t b = pixels[i * 4 + 0];
        uint8_t g = pixels[i * 4 + 1];
        uint8_t r = pixels[i * 4 + 2];
        
        // Use the brightness as alpha
        uint8_t alpha = r; // Since it's white text, R=G=B

        textureData[i * 4 + 0] = 255; // R (White)
        textureData[i * 4 + 1] = 255; // G (White)
        textureData[i * 4 + 2] = 255; // B (White)
        textureData[i * 4 + 3] = alpha; // Alpha from intensity
    }

    // Create D3D11 Texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = textureWidth;
    desc.Height = textureHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = textureData.data();
    initData.SysMemPitch = textureWidth * 4;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, &tex);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        DeleteObject(hFont);
        DeleteDC(hdc);
        RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);
        throw std::runtime_error("Failed to create D3D11 font texture");
    }

    // Create SRV
    FontData result;
    hr = device->CreateShaderResourceView(tex.Get(), nullptr, &result.texture);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        DeleteObject(hFont);
        DeleteDC(hdc);
        RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);
        throw std::runtime_error("Failed to create font SRV");
    }

    result.glyphs = glyphs;

    // Cleanup
    DeleteObject(hBitmap);
    DeleteObject(hFont);
    DeleteDC(hdc);
    RemoveFontResourceEx(filePath.c_str(), FR_PRIVATE, 0);

    return result;
}

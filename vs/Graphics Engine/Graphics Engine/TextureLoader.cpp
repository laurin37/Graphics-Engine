#include "EnginePCH.h"
#include "TextureLoader.h"
#include "Graphics.h" 

#include <wincodec.h>
#include <wrl/client.h>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")

namespace wrl = Microsoft::WRL;

wrl::ComPtr<ID3D11ShaderResourceView> TextureLoader::Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filename)
{
    wrl::ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) throw std::runtime_error("Failed to create WIC Factory");

    wrl::ComPtr<IWICBitmapDecoder> wicDecoder;
    hr = wicFactory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
    if (FAILED(hr)) throw std::runtime_error("Failed to load texture file");

    wrl::ComPtr<IWICBitmapFrameDecode> wicFrame;
    ThrowIfFailed(wicDecoder->GetFrame(0, &wicFrame));

    wrl::ComPtr<IWICFormatConverter> wicConverter;
    ThrowIfFailed(wicFactory->CreateFormatConverter(&wicConverter));
    ThrowIfFailed(wicConverter->Initialize(wicFrame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeMedianCut));

    UINT width, height;
    ThrowIfFailed(wicConverter->GetSize(&width, &height));

    std::vector<BYTE> buffer(static_cast<size_t>(width) * height * 4);
    ThrowIfFailed(wicConverter->CopyPixels(nullptr, width * 4, static_cast<UINT>(buffer.size()), buffer.data()));

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    wrl::ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &texture));

    context->UpdateSubresource(texture.Get(), 0, nullptr, buffer.data(), width * 4, 0);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    wrl::ComPtr<ID3D11ShaderResourceView> textureSRV;
    ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), &srvDesc, &textureSRV));

    context->GenerateMips(textureSRV.Get());

    return textureSRV;
}

// --- Minimal Procedural Font Generator ---
void DrawCharToBuffer(std::vector<uint32_t>& buffer, int texWidth, int charCode, const char* pattern)
{
    // Standard ASCII 16x16 Grid
    int cellX = (charCode % 16) * 16;
    int cellY = (charCode / 16) * 16;

    // Simple 5x7 font pattern logic inside 16x16 cell
    for (int y = 0; y < 7; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (pattern[y * 5 + x] == '#')
            {
                // Write white pixel
                int px = cellX + 5 + x; // Offset to center in cell
                int py = cellY + 4 + y;
                if (px < texWidth && py < texWidth)
                    buffer[py * texWidth + px] = 0xFFFFFFFF;
            }
        }
    }
}

wrl::ComPtr<ID3D11ShaderResourceView> TextureLoader::CreateDebugFont(ID3D11Device* device, ID3D11DeviceContext* context)
{
    const int width = 256;
    const int height = 256;
    std::vector<uint32_t> pixelData(width * height, 0x00000000); // Transparent background

    // Crude 5x7 Pixel Font Data
    const char* digits[] = {
        " ### " "#   #" "#   #" "#   #" " ### ", // 0
        "  #  " " ##  " "  #  " "  #  " " ### ", // 1
        " ### " "    #" " ### " "#    " " ### ", // 2
        " ### " "    #" " ### " "    #" " ### ", // 3
        "   #" "  ## " " # #" "#####" "   # ", // 4
        "#####" "#    " "#### " "    #" "#### ", // 5
        " ### " "#    " "#### " "#   #" " ### ", // 6
        "#####" "   # " "  #  " " #   " "#    ", // 7
        " ### " "#   #" " ### " "#   #" " ### ", // 8
        " ### " "#   #" " ####" "    #" " ### ", // 9
    };

    // Map Digits 0-9 (ASCII 48-57)
    for (int i = 0; i < 10; ++i) DrawCharToBuffer(pixelData, width, 48 + i, digits[i]);

    // Map 'F' (70)
    DrawCharToBuffer(pixelData, width, 70, "#####" "#    " "#### " "#    " "#    ");
    // Map 'P' (80)
    DrawCharToBuffer(pixelData, width, 80, "#### " "#   #" "#### " "#    " "#    ");
    // Map 'S' (83)
    DrawCharToBuffer(pixelData, width, 83, " ####" "#    " " ### " "    #" "#### ");
    // Map ':' (58)
    DrawCharToBuffer(pixelData, width, 58, "     " "  #  " "     " "  #  " "     ");

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = pixelData.data();
    sd.SysMemPitch = width * 4;

    wrl::ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(device->CreateTexture2D(&texDesc, &sd, &texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    wrl::ComPtr<ID3D11ShaderResourceView> textureSRV;
    ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), &srvDesc, &textureSRV));

    return textureSRV;
}
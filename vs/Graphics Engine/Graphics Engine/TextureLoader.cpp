#include "EnginePCH.h"
#include "TextureLoader.h"
#include "Graphics.h" // For ThrowIfFailed

#include <wincodec.h>
#include <wrl/client.h>

#pragma comment(lib, "windowscodecs.lib")

namespace wrl = Microsoft::WRL;

wrl::ComPtr<ID3D11ShaderResourceView> TextureLoader::Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& filename)
{
    // 1. Initialize WIC
    wrl::ComPtr<IWICImagingFactory> wicFactory;
    ThrowIfFailed(CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    ));

    // 2. Create a decoder
    wrl::ComPtr<IWICBitmapDecoder> wicDecoder;
    ThrowIfFailed(wicFactory->CreateDecoderFromFilename(
        filename.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &wicDecoder
    ));

    // 3. Get the frame from the image
    wrl::ComPtr<IWICBitmapFrameDecode> wicFrame;
    ThrowIfFailed(wicDecoder->GetFrame(0, &wicFrame));

    // 4. Convert the image format to 32bpp RGBA
    wrl::ComPtr<IWICFormatConverter> wicConverter;
    ThrowIfFailed(wicFactory->CreateFormatConverter(&wicConverter));
    ThrowIfFailed(wicConverter->Initialize(
        wicFrame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeMedianCut
    ));

    // 5. Get image dimensions
    UINT width, height;
    ThrowIfFailed(wicConverter->GetSize(&width, &height));

    // 6. Copy pixels to a buffer
    std::vector<BYTE> buffer(static_cast<size_t>(width) * height * 4);
    ThrowIfFailed(wicConverter->CopyPixels(
        nullptr,
        width * 4,
        static_cast<UINT>(buffer.size()),
        buffer.data()
    ));

    // 7. Create D3D11 Texture2D
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 0; // Generate all mip levels
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    wrl::ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, &texture));

    // 8. Update the texture with the image data
    context->UpdateSubresource(texture.Get(), 0, nullptr, buffer.data(), width * 4, 0);

    // 9. Create Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1; // Use all mip levels

    wrl::ComPtr<ID3D11ShaderResourceView> textureSRV;
    ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), &srvDesc, &textureSRV));

    // 10. Generate Mipmaps
    context->GenerateMips(textureSRV.Get());

    return textureSRV;
}
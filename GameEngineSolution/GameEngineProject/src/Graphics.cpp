#include "include/EnginePCH.h"
#include "include/Graphics.h"
#include "include/GameObject.h"
#include "include/Camera.h"
#include "include/Mesh.h"
#include "include/Material.h"
#include "include/Skybox.h"
#include "include/PostProcess.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

Graphics::Graphics()
    : m_screenWidth(0), m_screenHeight(0)
{
}

Graphics::~Graphics() = default;

void Graphics::Initialize(HWND hwnd, int width, int height)
{
    m_screenWidth = static_cast<float>(width);
    m_screenHeight = static_cast<float>(height);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_deviceContext
    ));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    m_backBufferTexture = backBuffer;
    ThrowIfFailed(m_device->CreateRenderTargetView(m_backBufferTexture.Get(), nullptr, &m_renderTargetView));

    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ThrowIfFailed(m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer));
    ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView));
}

Microsoft::WRL::ComPtr<ID3D11Device> Graphics::GetDevice() const { return m_device; }
Microsoft::WRL::ComPtr<ID3D11DeviceContext> Graphics::GetContext() const { return m_deviceContext; }
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Graphics::GetRenderTargetView() const { return m_renderTargetView; }
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Graphics::GetDepthStencilView() const { return m_depthStencilView; }
Microsoft::WRL::ComPtr<ID3D11Texture2D> Graphics::GetBackBuffer() const { return m_backBufferTexture; }
float Graphics::GetScreenWidth() const { return m_screenWidth; }
float Graphics::GetScreenHeight() const { return m_screenHeight; }

void Graphics::Present()
{
    // Present(0, 0) = VSync OFF (uncapped FPS)
    // Present(1, 0) = VSync ON (capped to monitor refresh rate)
    ThrowIfFailed(m_swapChain->Present(0, 0));
}

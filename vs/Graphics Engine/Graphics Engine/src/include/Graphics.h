#pragma once

#include "EnginePCH.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Shader.h"

// Forward Declarations
class Mesh;
class Camera;
class GameObject;
class Skybox;
class PostProcess;

// --- Lighting Structs ---
struct DirectionalLight
{
    DirectX::XMFLOAT4 direction; // w = unused
    DirectX::XMFLOAT4 color;     // w = intensity
};

struct PointLight
{
    DirectX::XMFLOAT4 position;    // w = range
    DirectX::XMFLOAT4 color;       // w = intensity
    DirectX::XMFLOAT4 attenuation; // x = constant, y = linear, z = quadratic
};

const int MAX_POINT_LIGHTS = 4;

// Constant buffer for vertex shader
struct CB_VS_vertexshader
{
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
    DirectX::XMFLOAT4X4 lightViewProjMatrix;
};

// Constant buffer for pixel shader (per-frame data)
struct CB_PS_Frame
{
    DirectionalLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    DirectX::XMFLOAT4 cameraPos;
};

class Graphics
{
public:
    Graphics();
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void Initialize(HWND hwnd, int width, int height);
    void Present();

    // --- Getters for Renderer ---
    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetContext() const;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> GetBackBuffer() const;

    float GetScreenWidth() const;
    float GetScreenHeight() const;

private:
    // Core D3D objects
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBufferTexture;

    // Depth/Stencil buffer
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    float m_screenWidth;
    float m_screenHeight;
};
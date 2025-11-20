#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <stdexcept>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory> // For std::unique_ptr
#include "Mesh.h"
#include "Camera.h"

// Helper for exception-based error handling
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception("DirectX Error");
    }
}

// Constant buffer for vertex shader
struct CB_VS_vertexshader
{
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
};

// Constant buffer for pixel shader
struct CB_PS_light
{
    DirectX::XMFLOAT4 lightDir;
    DirectX::XMFLOAT4 lightColor;
    DirectX::XMFLOAT4 cameraPos;
    float specularIntensity;
    float specularPower;
    float padding[2]; // Pad to a 16-byte boundary
};

class Graphics
{
public:
    Graphics();
    ~Graphics() = default;

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    Graphics(Graphics&&) = delete;
    Graphics& operator=(Graphics&&) = delete;

    void Initialize(HWND hwnd, int width, int height);
    void RenderFrame();
    Camera* GetCamera();

private:
    void InitPipeline();

    // Core D3D objects
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    // Depth/Stencil buffer
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    // Pipeline objects
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_psConstantBuffer;

    // Texturing objects
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    
    // Scene objects
    std::unique_ptr<Mesh> m_cubeMesh;
    std::unique_ptr<Camera> m_camera;

    // Matrices
    DirectX::XMMATRIX m_worldMatrix;
    DirectX::XMMATRIX m_projectionMatrix;

    // Animation
    float m_rotation;
};

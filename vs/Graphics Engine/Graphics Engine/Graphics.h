#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <stdexcept>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// Helper for exception-based error handling, as per GEMINI.md
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception("DirectX Error");
    }
}

// Vertex structure for the pipeline
struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
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

private:
    void InitPipeline();

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    // Pipeline objects
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
};

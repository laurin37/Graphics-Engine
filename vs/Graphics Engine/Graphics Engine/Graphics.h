#pragma once

#include "EnginePCH.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

// Forward Declarations
class Mesh;
class Camera;
class GameObject;

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
    DirectX::XMFLOAT4 lightDir;
    DirectX::XMFLOAT4 lightColor;
    DirectX::XMFLOAT4 cameraPos;
};

class Graphics
{
public:
    const int SHADOW_MAP_SIZE = 2048;

    Graphics();
    ~Graphics() = default;

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void Initialize(HWND hwnd, int width, int height);
    void RenderFrame(Camera* camera, const std::vector<std::unique_ptr<GameObject>>& gameObjects);
    
    Mesh* GetMeshAsset() const;

private:
    void InitPipeline();
    void RenderShadowPass(const std::vector<std::unique_ptr<GameObject>>& gameObjects, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj);
    void RenderMainPass(Camera* camera, const std::vector<std::unique_ptr<GameObject>>& gameObjects, const DirectX::XMMATRIX& lightViewProj);

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
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_psFrameConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_psMaterialConstantBuffer;

    // Texturing objects
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    
    // Shadow Mapping objects
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_shadowMapTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDSV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_shadowSampler;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shadowVS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbShadowMatrix;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_shadowRS;

    // Scene Assets
    std::unique_ptr<Mesh> m_meshAsset;

    // Matrices
    DirectX::XMMATRIX m_projectionMatrix;
};

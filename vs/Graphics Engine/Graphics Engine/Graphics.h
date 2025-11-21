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

// --- UI Structures ---
struct SpriteVertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT4 color;
};

struct CB_VS_UI
{
    DirectX::XMFLOAT2 screenSize;
    DirectX::XMFLOAT2 padding;
};

class Graphics
{
public:
    const int SHADOW_MAP_SIZE = 2048;

    Graphics();
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void Initialize(HWND hwnd, int width, int height);

    // Changed: RenderFrame no longer presents automatically
    void RenderFrame(
        Camera* camera,
        const std::vector<std::unique_ptr<GameObject>>& gameObjects,
        const DirectionalLight& dirLight,
        const std::vector<PointLight>& pointLights
    );

    // New: Call this at the very end of your loop
    void Present();

    // --- UI Methods ---
    void EnableUIState();
    void DisableUIState();
    void DrawUI(const SpriteVertex* vertices, size_t count, ID3D11ShaderResourceView* texture);

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetContext() const;
    Mesh* GetMeshAsset() const;

private:
    void InitPipeline();
    void RenderShadowPass(const std::vector<std::unique_ptr<GameObject>>& gameObjects, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj);
    void RenderMainPass(
        Camera* camera,
        const std::vector<std::unique_ptr<GameObject>>& gameObjects,
        const DirectX::XMMATRIX& lightViewProj,
        const DirectionalLight& dirLight,
        const std::vector<PointLight>& pointLights
    );

    // Core D3D objects
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    // Depth/Stencil buffer
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    // Pipeline objects
    std::unique_ptr<VertexShader> m_mainVS;
    std::unique_ptr<PixelShader> m_mainPS;
    std::unique_ptr<VertexShader> m_shadowVS;
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
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbShadowMatrix;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_shadowRS;

    // UI Objects
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_uiVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_uiPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_uiBlendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_uiDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_uiRS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_uiInputLayout;
    float m_screenWidth;
    float m_screenHeight;

    // Scene Assets
    std::unique_ptr<Mesh> m_meshAsset;
    std::unique_ptr<Skybox> m_skybox;

    // Matrices
    DirectX::XMMATRIX m_projectionMatrix;
};
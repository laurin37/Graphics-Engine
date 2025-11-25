#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "../Utils/EnginePCH.h"
#include "../Physics/Collision.h"

// Forward Declarations
class Graphics;
class Camera;
class VertexShader;
class PixelShader;
class Skybox;
class PostProcess;
class Mesh;
class Material;
struct DirectionalLight;
struct PointLight;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
class AssetManager; // Forward Declaration

// ==================================================================================
// Renderer Class
// ----------------------------------------------------------------------------------
// Handles the low-level rendering pipeline using DirectX 11.
// Responsible for:
// - Managing D3D11 resources (Shaders, Buffers, Textures)
// - Implementing the rendering passes (Shadow Pass -> Main Pass -> Post Process)
// - Drawing meshes with materials and lighting
// - Debug rendering (Wireframe AABBs)
// ==================================================================================
class Renderer
{
public:
    struct RenderInstance
    {
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
        AABB worldAABB{};
        bool hasBounds = false;
    };

    Renderer();
    ~Renderer();

    void Initialize(Graphics* graphics, AssetManager* assetManager, int width, int height);
    void RenderFrame(
        const Camera& camera,
        const std::vector<const RenderInstance*>& instances,
        const DirectionalLight& dirLight,
        const std::vector<PointLight>& pointLights
    );

    void RenderDebugAABBs(
        const Camera& camera,
        const std::vector<AABB>& aabbs);

    // Accessors
    PostProcess* GetPostProcess() { return m_postProcess.get(); }

private:
    void InitPipeline(int width, int height);
    void RenderShadowPass(const std::vector<const RenderInstance*>& instances, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj);
    void RenderMainPass(
        const Camera& camera,
        const std::vector<const RenderInstance*>& instances,
        const DirectX::XMMATRIX& lightViewProj,
        const DirectionalLight& dirLight,
        const std::vector<PointLight>& pointLights
    );

    Graphics* m_graphics = nullptr; // Non-owning pointer
    AssetManager* m_assetManager = nullptr; // Non-owning pointer

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

    // Debug Drawing objects
    std::unique_ptr<VertexShader> m_debugVS;
    std::unique_ptr<PixelShader> m_debugPS;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframeRS;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledDSS;
    
    // Scene objects
    std::unique_ptr<Skybox> m_skybox;
    std::unique_ptr<PostProcess> m_postProcess;
    
    // Matrices
    DirectX::XMMATRIX m_projectionMatrix;
};

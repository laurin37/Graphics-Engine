#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "../Utils/EnginePCH.h"

// Forward Declarations
class Graphics;
class Camera;
class GameObject;
class VertexShader;
class PixelShader;
class Skybox;
class PostProcess;
struct DirectionalLight;
struct PointLight;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
class AssetManager; // Forward Declaration

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(Graphics* graphics, AssetManager* assetManager, int width, int height);
    void RenderFrame(
        const Camera& camera,
        const std::vector<std::unique_ptr<GameObject>>& gameObjects,
        const DirectionalLight& dirLight,
        const std::vector<PointLight>& pointLights
    );
    void RenderDebug(
        const Camera& camera,
        const std::vector<std::unique_ptr<GameObject>>& gameObjects);

    // Accessors
    PostProcess* GetPostProcess() { return m_postProcess.get(); }

private:
    void InitPipeline(int width, int height);
    void RenderShadowPass(const std::vector<std::unique_ptr<GameObject>>& gameObjects, DirectX::XMMATRIX& outLightView, DirectX::XMMATRIX& outLightProj);
    void RenderMainPass(
        const Camera& camera,
        const std::vector<std::unique_ptr<GameObject>>& gameObjects,
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
    
    // Scene objects
    std::unique_ptr<Skybox> m_skybox;
    std::unique_ptr<PostProcess> m_postProcess;
    
    // Matrices
    DirectX::XMMATRIX m_projectionMatrix;
};

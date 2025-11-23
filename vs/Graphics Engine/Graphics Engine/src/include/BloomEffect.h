#pragma once
#include "EnginePCH.h"
#include <memory>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <d3d11.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Buffer;
class VertexShader;
class PixelShader;

/// <summary>
/// Bloom post-processing effect.
/// Extracts bright pixels, applies Gaussian blur, and blends with scene.
/// </summary>
class BloomEffect
{
public:
    BloomEffect();
    ~BloomEffect();

    BloomEffect(const BloomEffect&) = delete;
    BloomEffect& operator=(const BloomEffect&) = delete;

    /// <summary>
    /// Initialize the Bloom effect with render targets and shaders.
    /// </summary>
    /// <param name="device">D3D11 Device</param>
    /// <param name="width">Render target width</param>
    /// <param name="height">Render target height</param>
    /// <param name="threshold">Brightness threshold for bloom (default: 1.0)</param>
    /// <param name="intensity">Bloom intensity multiplier (default: 0.04)</param>
    void Init(ID3D11Device* device, int width, int height, float threshold = 1.0f, float intensity = 0.04f);

    /// <summary>
    /// Apply bloom effect to the source texture.
    /// </summary>
    /// <param name="context">D3D11 Device Context</param>
    /// <param name="sourceSRV">HDR scene texture to process</param>
    /// <returns>Shader resource view containing blurred bloom texture</returns>
    ID3D11ShaderResourceView* Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sourceSRV);

    // Parameter accessors
    void SetThreshold(float threshold) { m_threshold = threshold; }
    void SetIntensity(float intensity) { m_intensity = intensity; }
    float GetThreshold() const { return m_threshold; }
    float GetIntensity() const { return m_intensity; }

private:
    // Bright pass extraction
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_brightPassTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_brightPassRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_brightPassSRV;

    // Blur textures (ping-pong between horizontal and vertical)
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_blurTexture1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_blurRTV1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_blurSRV1;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_blurTexture2;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_blurRTV2;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_blurSRV2;

    // Shaders
    std::unique_ptr<VertexShader> m_fullscreenVS;
    std::unique_ptr<PixelShader> m_brightPassPS;
    std::unique_ptr<PixelShader> m_blurPS;

    // Sampler
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

    // Constant buffer for blur parameters
    struct BlurParams
    {
        DirectX::XMFLOAT2 direction; // (1,0) for horizontal, (0,1) for vertical
        float threshold;
        float padding;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_blurParamsCB;

    // Parameters
    float m_threshold;
    float m_intensity;
    int m_width;
    int m_height;
};

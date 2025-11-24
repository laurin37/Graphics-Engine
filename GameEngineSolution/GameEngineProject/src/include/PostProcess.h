#pragma once
#include "EnginePCH.h"
#include <memory>
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
class VertexShader;
class PixelShader;
class BloomEffect;

class PostProcess
{
public:
    PostProcess();
    ~PostProcess();

    PostProcess(const PostProcess&) = delete;
    PostProcess& operator=(const PostProcess&) = delete;

    void Init(ID3D11Device* device, int width, int height);
    void Bind(ID3D11DeviceContext* context, ID3D11DepthStencilView* dsv);
    void Draw(ID3D11DeviceContext* context, ID3D11RenderTargetView* backBufferRTV);

    // Bloom control
    void ToggleBloom() { m_bloomEnabled = !m_bloomEnabled; }
    void SetBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }
    bool IsBloomEnabled() const { return m_bloomEnabled; }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_offScreenTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_offScreenRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_offScreenSRV;
    std::unique_ptr<VertexShader> m_vs;
    std::unique_ptr<PixelShader> m_ps;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler; // Sampler for the scene texture.
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsState;
    
    // Bloom effect
    std::unique_ptr<BloomEffect> m_bloomEffect;
    bool m_bloomEnabled = true; // Toggle for bloom on/off
    
    // Black texture to use when bloom is disabled
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_blackTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_blackSRV;
};

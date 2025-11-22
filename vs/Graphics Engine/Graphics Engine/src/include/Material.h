#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "EnginePCH.h"

// This struct must match the CB_PS_Material in the Pixel Shader
struct CBuffer_PS_Material
{
	DirectX::XMFLOAT4 color;
	float specularIntensity;
	float specularPower;
	float padding[2];
};

class Material
{
public:
	Material(
		DirectX::XMFLOAT4 color,
		float specIntensity,
		float specPower,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV = nullptr,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV = nullptr
	);
	~Material() = default;

	void Bind(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const;

private:
	CBuffer_PS_Material m_data;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalSRV;
};

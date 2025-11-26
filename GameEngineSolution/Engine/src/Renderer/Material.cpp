#include "../../include/Renderer/Material.h"
#include "../../include/Utils/EnginePCH.h"


Material::Material(
	DirectX::XMFLOAT4 color, 
	float specIntensity, 
	float specPower,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV
)
	: m_textureSRV(textureSRV)
	, m_normalSRV(normalSRV)
{
	m_data.color = color;
	m_data.specularIntensity = specIntensity;
	m_data.specularPower = specPower;
}

void Material::Bind(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const
{
	// Bind the material properties
	context->UpdateSubresource(psMaterialConstantBuffer, 0, nullptr, &m_data, 0, 0);
	context->PSSetConstantBuffers(1, 1, &psMaterialConstantBuffer);

	// Bind the diffuse texture only if it exists
	if (m_textureSRV)
	{
		context->PSSetShaderResources(0, 1, m_textureSRV.GetAddressOf());
	}

	// Bind the normal map only if it exists
	if (m_normalSRV)
	{
		context->PSSetShaderResources(1, 1, m_normalSRV.GetAddressOf());
	}
}

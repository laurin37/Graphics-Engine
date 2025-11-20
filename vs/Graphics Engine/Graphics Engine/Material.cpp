#include "Material.h"

Material::Material(DirectX::XMFLOAT4 color, float specIntensity, float specPower)
{
    m_data.color = color;
    m_data.specularIntensity = specIntensity;
    m_data.specularPower = specPower;
}

void Material::Bind(ID3D11DeviceContext* context, ID3D11Buffer* psMaterialConstantBuffer) const
{
    context->UpdateSubresource(psMaterialConstantBuffer, 0, nullptr, &m_data, 0, 0);
    context->PSSetConstantBuffers(1, 1, &psMaterialConstantBuffer);
}

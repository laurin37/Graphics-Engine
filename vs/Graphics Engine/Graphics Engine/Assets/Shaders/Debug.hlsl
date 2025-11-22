cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
};

float4 VS(float3 pos : POSITION) : SV_POSITION
{
    return mul(float4(pos, 1.0f), worldViewProj);
}

float4 PS() : SV_TARGET
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f); // Bright Green
}

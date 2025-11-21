// Shaders/Skybox.hlsl

cbuffer CBuffer_Skybox : register(b0)
{
    matrix worldViewProj;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_INPUT VS_main(VS_INPUT input)
{
    PS_INPUT output;
    output.uv = input.uv;
    
    // Transform the vertex position
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    
    // Force the vertex to the far clipping plane
    output.pos.z = output.pos.w;
    
    return output;
}

Texture2D g_skyboxTexture : register(t0);
SamplerState g_sampler : register(s0);

float4 PS_main(PS_INPUT input) : SV_TARGET
{
    return g_skyboxTexture.Sample(g_sampler, input.uv);
}

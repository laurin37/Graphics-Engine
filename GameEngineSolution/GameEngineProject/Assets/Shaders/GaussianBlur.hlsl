// GaussianBlur.hlsl
// Separable Gaussian blur (horizontal or vertical based on direction parameter)

Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);

cbuffer BlurParams : register(b0)
{
    float2 direction;   // (1/width, 0) for horizontal, (0, 1/height) for vertical
    float threshold;    // Not used in blur
    float padding;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// 9-tap Gaussian weights (sigma ≈ 2.0)
// These sum to approximately 1.0
static const float weights[9] = 
{
    0.05, 0.09, 0.12, 0.15, 0.16, 0.15, 0.12, 0.09, 0.05
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    
    // Sample in a line (horizontal or vertical depending on direction)
    for (int i = -4; i <= 4; i++)
    {
        float2 offset = direction * float(i);
        float3 sample = inputTexture.Sample(inputSampler, input.uv + offset).rgb;
        color += sample * weights[i + 4];
    }
    
    return float4(color, 1.0);
}

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    
    // Debug line removed
    // if (texColor.a < 0.1) return float4(1, 0, 0, 0.5) * input.color; 
    
    return texColor * input.color;
}

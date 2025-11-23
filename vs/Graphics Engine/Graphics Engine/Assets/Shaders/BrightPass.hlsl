// BrightPass.hlsl
// Extracts bright pixels for bloom effect

Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

cbuffer BlurParams : register(b0)
{
    float2 direction;   // Not used in bright pass
    float threshold;    // Brightness threshold
    float padding;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // Sample the scene
    float3 color = sceneTexture.Sample(sceneSampler, input.uv).rgb;
    
    // Calculate perceived brightness using luminance
    // Using Rec. 709 luma coefficients
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    // Only keep pixels above threshold
    if (brightness > threshold)
    {
        // Return the bright pixel
        return float4(color, 1.0);
    }
    
    // Return black for pixels below threshold
    return float4(0.0, 0.0, 0.0, 1.0);
}

cbuffer ConstantBuffer : register(b0)
{
    float2 screenSize;
    float2 padding;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    // Convert Screen Pixels to NDC (-1 to 1)
    output.pos.x = (input.pos.x / screenSize.x) * 2.0 - 1.0;
    output.pos.y = -((input.pos.y / screenSize.y) * 2.0 - 1.0);
    
    // FIX: Set Z to 0.1 instead of 0.0 to avoid Near-Plane clipping
    output.pos.z = 0.1; 
    output.pos.w = 1.0;

    output.uv = input.uv;
    output.color = input.color;
    return output;
}

cbuffer LightMatrixBuffer : register(b0)
{
    matrix lightWVP;
}

// Only need position for depth pass
float4 main(float3 pos : POSITION) : SV_POSITION
{
    return mul(float4(pos, 1.0f), lightWVP);
}

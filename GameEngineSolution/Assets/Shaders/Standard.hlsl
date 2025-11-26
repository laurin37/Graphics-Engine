cbuffer ConstantBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewProjMatrix;
}
struct VS_INPUT { float3 pos : POSITION; float2 uv : TEXCOORD; float3 normal : NORMAL; float3 tangent : TANGENT; };
struct PS_INPUT { 
    float4 pos : SV_POSITION; 
    float2 uv : TEXCOORD; 
    float3 normal : NORMAL; 
    float3 tangent : TANGENT;
    float3 worldPos : WORLD_POS;
    float4 lightSpacePos : TEXCOORD1;
};

PS_INPUT VS_main(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.pos, 1.0f), worldMatrix);
    output.worldPos = worldPos.xyz;
    
    output.pos = mul(worldPos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);
    
    output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3)worldMatrix));
    output.uv = input.uv;
    output.lightSpacePos = mul(worldPos, lightViewProjMatrix);
    return output;
}

Texture2D g_texture : register(t0);
Texture2D g_normalMap : register(t1);
Texture2D g_shadowMap : register(t2);

SamplerState g_sampler : register(s0);
SamplerComparisonState g_shadowSampler : register(s2);


cbuffer FrameData : register(b0)
{
    // Directional Light
    float4 dirLightDirection;
    float4 dirLightColor;

    // Point Lights
    float4 pointLightPos[4];
    float4 pointLightColor[4];
    float4 pointLightAtt[4];

    // Camera
    float4 cameraPos;
}

cbuffer MaterialData : register(b1)
{
    float4 surfaceColor;
    float specularIntensity;
    float specularPower;
}

float3 CalcLighting(float3 litColor, float3 pixelNormal, float3 lightVec, float3 lightColor, float3 viewDir, float specIntensity, float specPower)
{
    float diffuseFactor = saturate(dot(pixelNormal, lightVec));
    float3 diffuse = litColor * diffuseFactor * lightColor;
    float3 halfVector = normalize(lightVec + viewDir);
    float specFactor = pow(saturate(dot(pixelNormal, halfVector)), specPower);
    float3 specular = specIntensity * specFactor * lightColor;
    return diffuse + specular;
}


float4 PS_main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normalMapSample = g_normalMap.Sample(g_sampler, input.uv).rgb;
    float3 pixelNormal;

    if (normalMapSample.b < 0.2) 
    {
         float height = normalMapSample.r;
         float dHdx = ddx(height);
         float dHdy = ddy(height);
         float3 dPdx = ddx(input.worldPos);
         float3 dPdy = ddy(input.worldPos);
         float3 surfGrad = (dHdx * dPdx + dHdy * dPdy) * 50.0f;
         pixelNormal = normalize(N - surfGrad);
    }
    else
    {
         float3 tangentSpaceNormal = normalize(normalMapSample * 2.0 - 1.0);
         pixelNormal = normalize(mul(tangentSpaceNormal, TBN));
    }

    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    float3 baseColor = texColor.rgb * surfaceColor.rgb;
    
    float shadowFactor = 0.0;
    float3 projCoords = input.lightSpacePos.xyz / input.lightSpacePos.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * -0.5 + 0.5;

    float shadowBias = 0.0005f;
    float texelSize = 1.0 / 2048.0;

    if (saturate(projCoords.z) > 0.0 && saturate(projCoords.x) > 0.0 && saturate(projCoords.x) < 1.0 && saturate(projCoords.y) > 0.0 && saturate(projCoords.y) < 1.0)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            [unroll]
            for (int y = -1; y <= 1; ++y)
            {
                shadowFactor += g_shadowMap.SampleCmpLevelZero(g_shadowSampler, projCoords.xy + float2(x, y) * texelSize, projCoords.z - shadowBias);
            }
        }
        shadowFactor /= 9.0;
    }
    else
    {
        shadowFactor = 1.0;
    }
    
    float3 finalColor = float3(0, 0, 0);
    float3 viewDir = normalize(cameraPos.xyz - input.worldPos);

    finalColor += baseColor * 0.15f;

    float3 dirLightContrib = CalcLighting(baseColor, pixelNormal, -dirLightDirection.xyz, dirLightColor.rgb * dirLightColor.a, viewDir, specularIntensity, specularPower);
    finalColor += dirLightContrib * shadowFactor;

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float3 lightVec = pointLightPos[i].xyz - input.worldPos;
        float dist = length(lightVec);
        
        if (dist < pointLightPos[i].w)
        {
            lightVec = normalize(lightVec);
            float3 pointLightContrib = CalcLighting(baseColor, pixelNormal, lightVec, pointLightColor[i].rgb * pointLightColor[i].a, viewDir, specularIntensity, specularPower);
            float att = 1.0 / (pointLightAtt[i].x + pointLightAtt[i].y * dist + pointLightAtt[i].z * dist * dist);
            finalColor += pointLightContrib * att;
        }
    }

    // Add emissive component (bright materials emit light directly)
    // Use material color's channel to determine emissive strength
    // If specular power is very high (>100), treat as emissive
    float emissiveStrength = (specularPower > 100.0f) ? 2.5f : 0.0f; // Increased for obvious glow
    float3 emissive = baseColor * emissiveStrength;
    finalColor += emissive;

    return float4(finalColor, texColor.a);
}

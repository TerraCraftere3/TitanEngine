#include "Library/Types.slang"

cbuffer GlobalUBO : register(b0)
{
    float3 u_ViewPosition;    // 12 bytes
    bool u_HasDirectionLight; // 4 bytes (bool is 4 bytes in HLSL)
    float3 u_LightDirection;  // 12 bytes
    float padding1;           // 4 bytes -> total = 96 bytes (aligned)
};

struct VertexInput
{
    float2 a_Position : POSITION;
    float2 a_TexCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct FragmentOutput
{
    float4 color : SV_Target0;
    int entityID : SV_Target1;
    float depth : SV_Depth;
};

[[vk::binding(0)]]
Sampler2D gPosition;

[[vk::binding(1)]]
Sampler2D gNormal;

[[vk::binding(2)]]
Sampler2D gAlbedo;

[[vk::binding(3)]]
Sampler2D gMetallicRoughness;

[[vk::binding(4)]]
Sampler2D gEmission;

[[vk::binding(5)]]
ISampler2D gEntityID;

[[vk::binding(6)]]
Sampler2D gDepth;

[[vk::binding(7)]]
SamplerCube IrradianceMap;

[shader("vertex")] VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.a_Position, 0.0, 1.0);
    output.texCoord = input.a_TexCoord;
    return output;
}

static const float PI = 3.14159265359;

[shader("fragment")] FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    // --- Sample G-buffer ---
    float3 position = gPosition.Sample(input.texCoord).xyz;
    float3 normal = gNormal.Sample(input.texCoord).xyz * 2.0 - 1.0; // convert from [0,1] -> [-1,1]

    float4 albedoColor = gAlbedo.Sample(input.texCoord);
    float3 emission = gEmission.Sample(input.texCoord).rgb;
    float4 mr = gMetallicRoughness.Sample(input.texCoord);
    float metallic = mr.r;
    float roughness = mr.g;
    float ao = mr.b;

    if (albedoColor.a == 0)
        discard;

    float3 N = normalize(normal);
    float3 V = normalize(u_ViewPosition - position);

    // --- IBL diffuse irradiance ---
    float3 irradiance = IrradianceMap.Sample(N).rgb;
    float3 color = irradiance * albedoColor.rgb * ao;

    // --- Directional lighting ---
    if (u_HasDirectionLight)
    {
        float3 L = normalize(-u_LightDirection);
        float3 H = normalize(V + L);

        // Fresnel-Schlick
        float3 F0 = float3(0.04, 0.04, 0.04);
        F0 = lerp(F0, albedoColor.rgb, metallic);
        float3 F = F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);

        // GGX Distribution
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float D = a2 / (PI * pow(NdotH * NdotH * (a2 - 1.0) + 1.0, 2.0) + 1e-5);

        // Schlick-GGX Geometry
        float k = (roughness + 1.0);
        k = (k * k) / 8.0;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float G_V = NdotV / (NdotV * (1.0 - k) + k);
        float G_L = NdotL / (NdotL * (1.0 - k) + k);
        float G = G_V * G_L;

        // Cook-Torrance BRDF
        float3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 1e-5;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        float3 diffuse = kD * albedoColor.rgb / PI;

        float3 radiance = float3(1.0, 1.0, 1.0);
        color += (diffuse + specular) * radiance * NdotL;
    }

    color += emission;

    output.color = float4(color, albedoColor.a);
    output.entityID = (int)gEntityID.Sample(input.texCoord).r;
    output.depth = gDepth.Sample(input.texCoord).r;

    return output;
}

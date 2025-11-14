cbuffer GlobalUBO : register(b0)
{
    float3 u_ViewPosition;                  // 12 bytes
    bool u_HasDirectionLight;               // 4 bytes (bool is 4 bytes in HLSL)
    float3 u_LightDirection;                // 12 bytes
    float padding1;                         // 4 bytes -> total = 96 bytes (aligned)
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

Sampler2D gPosition;
Sampler2D gNormal;
Sampler2D gAlbedo;
Sampler2D gMetallicRoughness;
Sampler2D gEntityID;
Sampler2D gDepth;


[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.a_Position, 0.0, 1.0);
    output.texCoord = input.a_TexCoord;
    return output;
}

static const float3 AMBIENT_LIGHT = float3(0.05f, 0.04f, 0.03f);
static const float PI = 3.14159265359;

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    float3 position = gPosition.Sample(input.texCoord).xyz;
    float3 normal = gNormal.Sample(input.texCoord).xyz;

    float4 albedoColor = gAlbedo.Sample(input.texCoord);
    float4 _ = gMetallicRoughness.Sample(input.texCoord);
    float metallic = _.r;
    float roughness = _.g;
    float ao = _.b;

    float3 N = normalize(normal);
    float3 V = normalize(u_ViewPosition - (input.position.xyz / input.position.w));

    float3 color = AMBIENT_LIGHT * albedoColor.rgb * ao;

    if (u_HasDirectionLight)
    {
        float3 L = normalize(-u_LightDirection);
        float3 H = normalize(V + L);

        // Fresnel-Schlick
        float3 F0 = float3(0.04, 0.04, 0.04);
        F0 = lerp(F0, albedoColor.rgb, metallic);
        float3 F = F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);

        // Distribution GGX
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float D = a2 / (PI * pow(NdotH * NdotH * (a2 - 1.0) + 1.0, 2.0) + 1e-5);

        // Geometry (Schlick-GGX)
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

    color = color / (color + float3(1.0));
    color = pow(color, float3(1.0 / 2.2));

    output.color = float4(color.rgb, albedoColor.a);
    output.entityID = -1;
    output.depth = gDepth.Sample(input.texCoord).r;

    return output;
}

cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 16-byte aligned, 64 bytes
    float3 u_ViewPosition;                  // 12 bytes
    float Padding0;                         // padding to 16 bytes
};

struct Material
{
    float4 Albedo;
    float Metallic;
    float Roughness;
    float2 Padding;
};

StructuredBuffer<Material> u_Materials : register(t1);

struct VertexInput
{
    float3 a_Position : POSITION;
    float3 a_Normal : NORMAL;
    float2 a_TexCoord : TEXCOORD0;
    int a_EntityID : TEXCOORD3;
    int a_MaterialIndex : TEXCOORD4;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : COLOR;
    float2 texCoord : TEXCOORD0;
    nointerpolation int entityID : TEXCOORD3;
    nointerpolation int materialIndex : TEXCOORD4;
};

struct FragmentOutput
{
    float4 color : SV_Target0;
    int entityID : SV_Target1;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(u_ViewProjection, float4(input.a_Position, 1.0));
    output.normal = input.a_Normal;
    output.texCoord = input.a_TexCoord;
    output.entityID = input.a_EntityID;
    output.materialIndex = input.a_MaterialIndex;
    return output;
}

static const float3 LIGHT_DIR = normalize(float3(-0.5, -1.0, -0.5));
static const float3 AMBIENT_LIGHT = float3(0.05f, 0.04f, 0.03f);
static const float PI = 3.14159265359;

float4 GET_ALBEDO_COLOR(Material mat, float2 texCoord)
{
    return mat.Albedo;
}

float GET_METALLIC(Material mat, float2 texCoord)
{
    return mat.Metallic;
}

float GET_ROUGHNESS(Material mat, float2 texCoord)
{
    return mat.Roughness;
}

float GET_AO(Material mat, float2 texCoord)
{
    return 1.0;
}

float DistributionGGX(float4 N, float4 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(float4 N, float4 V, float4 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    // Fetch material
    Material mat = u_Materials[input.materialIndex];
    float4 albedoColor = GET_ALBEDO_COLOR(mat, input.texCoord);
    float metallic = GET_METALLIC(mat, input.texCoord);
    float roughness = GET_ROUGHNESS(mat, input.texCoord);
    float ao = GET_AO(mat, input.texCoord);

    // Normal and view direction
    float3 N = normalize(input.normal);
    float3 V = normalize(u_ViewPosition - (input.position.xyz / input.position.w));

    // Light
    float3 L = normalize(-LIGHT_DIR);
    float3 H = normalize(V + L);

    // Fresnel (Schlick approximation)
    float3 F0 = float3(0.04, 0.04, 0.04); // dielectric base
    F0 = lerp(F0, albedoColor.rgb, metallic);
    float3 F = F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);

    // Distribution (GGX)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denomD = NdotH2 * (a2 - 1.0) + 1.0;
    float D = a2 / (PI * denomD * denomD + 1e-5);

    // Geometry (Smith / Schlick-GGX)
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

    // kS = specular, kD = diffuse
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);

    // Lambertian diffuse
    float3 diffuse = kD * albedoColor.rgb / PI;

    // Combine
    float3 radiance = float3(1.0, 1.0, 1.0); // directional light intensity
    float3 Lo = (diffuse + specular) * radiance * NdotL;

    // Ambient
    float3 ambient = AMBIENT_LIGHT * albedoColor.rgb * ao;

    float3 color = ambient + Lo;

    // Tone mapping (ACES-like) and gamma
    color = color / (color + float3(1.0));
    color = pow(color, float3(1.0 / 2.2));

    output.color = float4(color, albedoColor.a);
    output.entityID = input.entityID;

    return output;
}

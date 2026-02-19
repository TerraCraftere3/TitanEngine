#include "Library/Functions.slang"
#include "Library/Types.slang"

[[vk::binding(0)]]
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 64 bytes
    column_major float4x4 u_Transform;      // 64 bytes
    uint32_t u_EntityID;                    // 4 bytes
    uint32_t u_LODIndex;                    // 4 bytes
    float4 AlbedoColor;                     // 16 bytes
    float2 UVRepeat;                        // 8 bytes
    float2 UV0;                             // 8 bytes
    float2 UV1;                             // 8 bytes
};

struct VertexInput
{
    float3 a_Position : POSITION;
    float3 a_Normal : NORMAL;
    float3 a_Tangent : TANGENT;
    float2 a_TexCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPosition : TEXCOORD5;
    float3 normal : COLOR;
    float3 tangent : TEXCOORD1;
    float3 bitangent : TEXCOORD2;
    float2 texCoord : TEXCOORD0;
    nointerpolation int entityID : TEXCOORD3;
};

struct FragmentOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_TARGET1;
    float4 albedo : SV_TARGET2;
    float4 metallic_roughness_ao_ : SV_TARGET3;
    float4 emission : SV_TARGET4;
    int entityID : SV_Target5;
};

[[vk::binding(0)]]
Sampler2D heightMap : register(s0);

[[vk::binding(1)]]
Sampler2D albedoMap : register(s1);

[[vk::binding(2)]]
Sampler2D emissionMap : register(s2);

[[vk::binding(3)]]
Sampler2D normalMap : register(s3);

[[vk::binding(4)]]
Sampler2D metallicMap : register(s4);

[[vk::binding(5)]]
Sampler2D roughnessMap : register(s5);

[[vk::binding(6)]]
Sampler2D aoMap : register(s6);

[shader("vertex")] VertexOutput vertexMain(VertexInput input, uint instanceID : SV_InstanceID) {
    VertexOutput output;

    float3 normal = input.a_Normal;
    float3 tangent = input.a_Tangent;
    float2 uv = (UV0 + input.a_TexCoord * UV1) * UVRepeat;

    // Sample height at current and neighboring positions
    float height = heightMap.Sample(uv).r;
    float heightRight = heightMap.Sample(uv + float2(0.01, 0.0)).r;
    float heightUp = heightMap.Sample(uv + float2(0.0, 0.01)).r;

    float3 displacedPosition = input.a_Position + normal * height;

    // Calculate normal from height map using finite differences
    float3 tangentDir = normalize(tangent);
    float3 bitangentDir = normalize(cross(normal, tangentDir));

    float3 posRight = input.a_Position + tangentDir * 0.01 + normal * heightRight;
    float3 posUp = input.a_Position + bitangentDir * 0.01 + normal * heightUp;

    float3 computedTangent = normalize(posRight - displacedPosition);
    float3 computedBitangent = normalize(posUp - displacedPosition);
    float3 computedNormal = normalize(cross(computedBitangent, computedTangent));

    float4 worldPos = mul(u_Transform, float4(displacedPosition, 1.0));
    output.position = mul(u_ViewProjection, worldPos);
    output.worldPosition = worldPos.xyz;

    output.normal = normalize(mul((float3x3)u_Transform, computedNormal));
    output.tangent = normalize(mul((float3x3)u_Transform, computedTangent));

    output.bitangent = cross(output.normal, output.tangent);

    output.texCoord = uv;
    output.entityID = u_EntityID;

    return output;
};

[shader("fragment")] FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;
    float2 uv = input.texCoord;

    float3 albedoSample = albedoMap.Sample(uv).rgb * AlbedoColor.rgb;
    float3 normalSample = normalMap.Sample(uv).rgb;
    float metallic = metallicMap.Sample(uv).r;
    float roughness = roughnessMap.Sample(uv).r;
    float ao = aoMap.Sample(uv).r;
    float3 emission = emissionMap.Sample(uv).rgb;

    // Reconstruct normal from tangent space
    float3 normal = normalize(normalSample * 2.0 - 1.0);
    float3 worldNormal = normalize(normal.x * input.tangent + normal.y * input.bitangent + normal.z * input.normal);

    // Output values from textures
    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(worldNormal, 1.0);
#ifdef DEBUG_LOD
    float3 lodRGB = wavelengthToRGB(380.0 + u_LODIndex * 40.0);
    output.albedo = float4(albedoSample * lodRGB, AlbedoColor.a);
#else
    output.albedo = float4(albedoSample, AlbedoColor.a);
#endif
    output.metallic_roughness_ao_ = float4(metallic, roughness, ao, 1.0);
    output.emission = float4(emission, 1.0);
    output.entityID = input.entityID;

    return output;
}

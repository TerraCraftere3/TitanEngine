#include "Library/Functions.slang"
#include "Library/Types.slang"

[[vk::binding(0)]]
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 64 bytes
    column_major float4x4 u_Transform;      // 64 bytes
    uint32_t u_EntityID;                       // 4 bytes
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

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint instanceID: SV_InstanceID)
{
    VertexOutput output;

    float3 normal = input.a_Normal;
    float3 tangent = input.a_Tangent;

    // Sample height at current and neighboring positions
    float height = heightMap.Sample(input.a_TexCoord).r;
    float heightRight = heightMap.Sample(input.a_TexCoord + float2(0.01, 0.0)).r;
    float heightUp = heightMap.Sample(input.a_TexCoord + float2(0.0, 0.01)).r;
    
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

    output.texCoord = input.a_TexCoord;
    output.entityID = u_EntityID;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    // Output raw values
    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(input.normal, 1.0);
    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.metallic_roughness_ao_ = float4(0.0f, 0.5f, 1.0f, 1.0f);
    output.emission = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.entityID = input.entityID;

    return output;
}

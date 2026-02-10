cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 64 bytes
};

struct Material
{
    float4 AlbedoColor; // 16 bytes

    uint2 AlbedoTextureIndex;    // 8 bytes
    uint2 EmissionTextureIndex;  // 8 bytes
    uint2 MetallicTextureIndex;  // 8 bytes
    uint2 AOTextureIndex;        // 8 bytes
    uint2 RoughnessTextureIndex; // 8 bytes
    uint2 NormalTextureIndex;    // 8 bytes

    float2 UVRepeat;
};

struct InstanceData
{
    column_major float4x4 Transform;    // 64 bytes
    column_major float4x4 NormalMatrix; // 64 bytes
    int EntityID;                       // 4 bytes
    int MaterialIndex;                  // 4 bytes
    float2 _pad;                        // 8 bytes padding
};

StructuredBuffer<Material> u_Materials : register(t1);
StructuredBuffer<InstanceData> u_Instances : register(t2);

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
    nointerpolation int materialIndex : TEXCOORD4;
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

static const float3 AMBIENT_LIGHT = float3(0.05f, 0.04f, 0.03f);
static const float PI = 3.14159265359;

float4 GET_ALBEDO_COLOR(Material mat, float2 texCoord)
{
    float3 color = mat.AlbedoColor.rgb;
    color *= GetBindlessTexture(mat.AlbedoTextureIndex).Sample(texCoord).rgb;
    return float4(color, 1.0);
}

float GET_METALLIC(Material mat, float2 texCoord)
{
    return GetBindlessTexture(mat.MetallicTextureIndex).Sample(texCoord).r;
}

float GET_ROUGHNESS(Material mat, float2 texCoord)
{
    return GetBindlessTexture(mat.RoughnessTextureIndex).Sample(texCoord).g;
}

float GET_AO(Material mat, float2 texCoord)
{
    return GetBindlessTexture(mat.AOTextureIndex).Sample(texCoord).r;
}

float3 GET_EMISSION(Material mat, float2 texCoord)
{
    return GetBindlessTexture(mat.EmissionTextureIndex).Sample(texCoord).rgb;
}

float3 GET_NORMAL(Material mat, VertexOutput input)
{
    float3 sampledNormal = GetBindlessTexture(mat.NormalTextureIndex).Sample(input.texCoord * mat.UVRepeat).rgb;
    sampledNormal = sampledNormal * 2.0f - 1.0f;

    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
    return normalize(mul(sampledNormal, TBN));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint instanceID: SV_InstanceID)
{
    VertexOutput output;

    InstanceData instance = u_Instances[instanceID];

    float3 normal = input.a_Normal;
    float3 tangent = input.a_Tangent;

    float4 worldPos = mul(instance.Transform, float4(input.a_Position, 1.0));
    output.position = mul(u_ViewProjection, worldPos);
    output.worldPosition = worldPos.xyz;

    output.normal = normalize(mul((float3x3)instance.NormalMatrix, normal));
    output.tangent = normalize(mul((float3x3)instance.Transform, tangent));

    output.bitangent = cross(output.normal, output.tangent);

    output.texCoord = input.a_TexCoord;
    output.entityID = instance.EntityID;
    output.materialIndex = instance.MaterialIndex;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    Material mat = u_Materials[input.materialIndex];
    float2 uv = input.texCoord * mat.UVRepeat;

    float4 albedoColor = GET_ALBEDO_COLOR(mat, uv);
    float metallic = GET_METALLIC(mat, uv);
    float roughness = GET_ROUGHNESS(mat, uv);
    float ao = GET_AO(mat, uv);
    float4 emission = float4(GET_EMISSION(mat, uv), 1.0);

    // Output raw values
    output.position = float4(input.worldPosition, 1.0);
    output.normal = float4(GET_NORMAL(mat, input), 1.0f);
    output.albedo = albedoColor;
    output.metallic_roughness_ao_ = float4(metallic, roughness, ao, 1.0f);
    output.emission = emission;
    output.entityID = input.entityID;

    return output;
}
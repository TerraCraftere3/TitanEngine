cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 64 bytes
};

struct Material
{
    float4 AlbedoColor; // 16 bytes

    uint2 AlbedoTextureIndex; // 8 bytes
    float Metallic;           // 4 bytes
    float Padding0;           // 4 bytes

    uint2 MetallicTextureIndex; // 8 bytes
    uint2 AOTextureIndex;
    uint2 RoughnessTextureIndex; // 8 bytes
    uint2 NormalTextureIndex;    // 8 bytes

    float2 UVRepeat;
};

StructuredBuffer<Material> u_Materials : register(t1);
StructuredBuffer<uint2> u_TextureHandles : register(t2);

struct VertexInput
{
    float3 a_Position : POSITION;
    float3 a_Normal : NORMAL;
    float3 a_Tangent : TANGENT;
    float2 a_TexCoord : TEXCOORD0;
    int a_EntityID : TEXCOORD3;
    int a_MaterialIndex : TEXCOORD4;
};

struct VertexOutput
{
    float4 position : SV_Position;
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
    float4 metallic_roughness__ : SV_TARGET3;
    int entityID : SV_Target4;
};

static const float3 AMBIENT_LIGHT = float3(0.05f, 0.04f, 0.03f);
static const float PI = 3.14159265359;

Sampler2D GetBindlessTexture(uint2 handle);

float4 GET_ALBEDO_COLOR(Material mat, float2 texCoord)
{
    float3 color = mat.AlbedoColor.rgb;

    color *= GetBindlessTexture(mat.AlbedoTextureIndex).Sample(texCoord).rgb;

    return float4(color, 1.0);
}

float GET_METALLIC(Material mat, float2 texCoord)
{
    float metallic = GetBindlessTexture(mat.MetallicTextureIndex).Sample(texCoord).r;

    return metallic;
}

float GET_ROUGHNESS(Material mat, float2 texCoord)
{
    float roughness = GetBindlessTexture(mat.RoughnessTextureIndex).Sample(texCoord).g;

    return roughness;
}

float GET_AO(Material mat, float2 texCoord)
{
    return GetBindlessTexture(mat.AOTextureIndex).Sample(texCoord).r;
}

float3 GET_NORMAL(Material mat, VertexOutput input)
{
    float3 sampledNormal = GetBindlessTexture(mat.NormalTextureIndex).Sample(input.texCoord * mat.UVRepeat).rgb;

    sampledNormal = sampledNormal * 2.0f - 1.0f;

    float3x3 TBN = float3x3(normalize(input.tangent),
                            normalize(input.bitangent),
                            normalize(input.normal));

    return normalize(mul(sampledNormal, TBN));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(u_ViewProjection, float4(input.a_Position, 1.0));
    output.normal = normalize(input.a_Normal);

    // Compute bitangent
    float3 n = normalize(input.a_Normal);
    float3 t = normalize(input.a_Tangent);
    float3 b = cross(n, t); // assuming tangent is orthogonal to normal

    output.tangent = t;
    output.bitangent = b;
    output.texCoord = input.a_TexCoord;
    output.entityID = input.a_EntityID;
    output.materialIndex = input.a_MaterialIndex;
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

    // Output raw values
    output.position = input.position;
    output.normal = float4(GET_NORMAL(mat, input), 1.0f);
    output.albedo = albedoColor;
    output.metallic_roughness__ = float4(metallic, roughness, ao, 1.0f);
    output.entityID = input.entityID;

    return output;
}
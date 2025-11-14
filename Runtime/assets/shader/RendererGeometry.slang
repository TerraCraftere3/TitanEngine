cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection; // 64 bytes
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
    float4 position : SV_Target0;
    float4 normal : SV_TARGET1;
    float4 albedo : SV_TARGET2;
    float4 metallic_roughness__ : SV_TARGET3;
    int entityID : SV_Target4;
};

static const float3 AMBIENT_LIGHT = float3(0.05f, 0.04f, 0.03f);
static const float PI = 3.14159265359;

// Material getters (placeholders for future texture sampling)
float4 GET_ALBEDO_COLOR(Material mat, float2 texCoord) { return mat.Albedo; }
float GET_METALLIC(Material mat, float2 texCoord) { return mat.Metallic; }
float GET_ROUGHNESS(Material mat, float2 texCoord) { return mat.Roughness; }
float GET_AO(Material mat, float2 texCoord) { return 1.0; }

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

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    Material mat = u_Materials[input.materialIndex];

    float4 albedoColor = GET_ALBEDO_COLOR(mat, input.texCoord);
    float metallic = GET_METALLIC(mat, input.texCoord);
    float roughness = GET_ROUGHNESS(mat, input.texCoord);

    // Output raw values
    output.position = input.position;
    output.normal = float4(normalize(input.normal), 1.0);
    output.albedo = albedoColor;
    output.metallic_roughness__ = float4(metallic, roughness, 0.0f, 1.0f);
    output.entityID = input.entityID;

    return output;
}

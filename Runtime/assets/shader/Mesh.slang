cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection;
};

struct Material
{
    float4 Albedo;
    float Metallic;
    float Roughness;
    float Padding[2]; // Pad to 16-byte alignment
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

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;
    Material mat = u_Materials[input.materialIndex];

    float NdotL = saturate(dot(normalize(input.normal), -LIGHT_DIR));

    float shadowFactor = 0.3 + 0.7 * NdotL;

    output.color = float4(shadowFactor * mat.Albedo.rgb, 1.0);
    output.entityID = input.entityID;

    return output;
}
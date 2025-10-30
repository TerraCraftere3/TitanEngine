// RendererQuad.slang

uniform column_major float4x4 u_ViewProjection;

struct VertexInput
{
    float3 a_Position : POSITION;
    float4 a_Color : COLOR;
    float2 a_UV : TEXCOORD0;
    float a_TexIndex : TEXCOORD1;
    float a_TilingFactor : TEXCOORD2;
    int a_EntityID : TEXCOORD3;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
    float texIndex : TEXCOORD1;
    float tilingFactor : TEXCOORD2;
    int entityID : TEXCOORD3;
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
    output.color = input.a_Color;
    output.texCoord = input.a_UV * input.a_TilingFactor;
    output.texIndex = input.a_TexIndex;
    output.tilingFactor = input.a_TilingFactor;
    output.entityID = input.a_EntityID;
    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;
    
    int texIndex = int(input.texIndex);
    
    output.color = input.color;
    output.entityID = input.entityID;
    
    return output;
}
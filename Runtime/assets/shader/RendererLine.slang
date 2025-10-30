// simple_color_shader.slang

// Uniforms
uniform column_major float4x4 u_ViewProjection;

// Vertex shader input
struct VertexInput
{
    float3 a_Position : POSITION;
    float4 a_Color : COLOR0;
    int a_EntityID : TEXCOORD0;
};

// Vertex shader output / Fragment shader input
struct VertexOutput
{
    float4 position : SV_Position;
    float4 v_Color : COLOR0;
    int v_EntityID : TEXCOORD0;
};

// Vertex Shader
[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.v_Color = input.a_Color;
    output.v_EntityID = input.a_EntityID;
    output.position = mul(u_ViewProjection, float4(input.a_Position, 1.0));

    return output;
}

// Fragment Shader
struct FragmentOutput
{
    float4 o_Color : SV_Target0;
    int o_EntityID : SV_Target1;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    output.o_Color = input.v_Color;
    output.o_EntityID = input.v_EntityID;

    return output;
}

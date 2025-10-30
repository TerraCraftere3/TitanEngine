// circle_shader.slang
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection;
};

// Vertex shader input
struct VertexInput
{
    float3 a_Position : POSITION;
    float3 a_LocalPosition : TEXCOORD0;
    float4 a_Color : COLOR0;
    float a_Thickness : TEXCOORD1;
    float a_Fade : TEXCOORD2;
    int a_EntityID : TEXCOORD3;
};

// Vertex shader output / Fragment shader input
struct VertexOutput
{
    float4 position : SV_Position;
    float3 v_LocalPosition : TEXCOORD0;
    float4 v_Color : COLOR0;
    float v_Thickness : TEXCOORD1;
    float v_Fade : TEXCOORD2;
    int v_EntityID : TEXCOORD3;
};

// Vertex Shader
[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.v_LocalPosition = input.a_LocalPosition;
    output.v_Color = input.a_Color;
    output.v_Thickness = input.a_Thickness;
    output.v_Fade = input.a_Fade;
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

    float distance = 1.0 - length(input.v_LocalPosition);

    float circle = smoothstep(0.0, input.v_Fade, distance);
    circle *= smoothstep(input.v_Thickness + input.v_Fade, input.v_Thickness, distance);

    if (circle == 0.0)
        discard;

    output.o_Color = input.v_Color;
    output.o_Color.a *= circle;

    output.o_EntityID = input.v_EntityID;

    return output;
}

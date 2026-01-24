[[vk::binding(0)]]
SamplerCube cubeMap;

[[vk::binding(0)]]
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_View;
    column_major float4x4 u_Projection;
    
    float3 u_TopColor;
    float3 u_BottomColor;
};

struct VertexInput {
    float3 a_Position : POSITION;
}

struct VertexOutput {
    float4 clipPos : SV_Position;
    float3 localPosition : TEXCOORD0;
}

struct FragmentOutput {
    float4 color : SV_Target0;
    int entityID : SV_Target1;
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.localPosition = input.a_Position;

    float3x3 r = (float3x3)u_View;

    float4x4 rotView = float4x4(
        float4(r[0], 0.0),
        float4(r[1], 0.0),
        float4(r[2], 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    output.clipPos = mul(u_Projection, mul(rotView, float4(output.localPosition, 1.0))); output.clipPos.z = output.clipPos.w;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    float3 envColor = cubeMap.Sample(normalize(input.localPosition)).rgb;

    //envColor = envColor / (envColor + float3(1.0));
    //envColor = pow(envColor, float3(1.0 / 2.2));

    output.color = float4(envColor, 1.0);
    output.entityID = -1;

    return output;
}
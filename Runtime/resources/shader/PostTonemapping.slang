#include "Library/Types.slang"

cbuffer TonemappingParams : register(b0)
{
    int u_Operator;
    float u_Exposure;
    float u_Gamma;      
    float u_WhitePoint;
}

[[vk::binding(0)]]
Sampler2D u_HDRInput : register(t0);
[[vk::binding(1)]]
ISampler2D u_Entity : register(t1);

struct VertexInput
{
    float2 a_Position : POSITION;
    float2 a_TexCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.Position = float4(input.a_Position, 0.0, 1.0);
    output.TexCoord = input.a_TexCoord;
    return output;
}


// ============================================================================
// Tonemapping Operators
// ============================================================================

float3 TonemapNone(float3 color)
{
    return color / (1.0 + color); // soft HDR compression
}

float3 TonemapFilmic(float3 color)
{
    float3 x = max(color - 0.004, 0.0);
    return (x * (6.2 * x + 0.5)) /
           (x * (6.2 * x + 1.7) + 0.06);
}


// ============================================================================
// Fragment Shader
// ============================================================================

struct FragmentOutput
{
    float4 color : SV_Target0;
    int entityID : SV_Target1;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput out;

    float3 hdrColor = u_HDRInput.Sample(input.TexCoord).rgb;

    hdrColor *= u_Exposure;

    float3 ldrColor;

    switch (u_Operator)
    {
        case 0: ldrColor = TonemapNone(hdrColor); break;
        case 1: ldrColor = TonemapFilmic(hdrColor); break;
        default: ldrColor = TonemapNone(hdrColor); break;
    }

    out.color = float4(saturate(ldrColor), 1.0);
    out.entityID = (int)u_Entity.Sample(input.TexCoord).r;
    return out;
}

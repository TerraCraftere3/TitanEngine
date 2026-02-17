#include "Library/SkyboxFunctions.slang"

static const float TIME_WRAP_MIN = -17.0;
static const float TIME_WRAP_MAX = 17.0;
static const float HORIZON_FADE_START = -0.1;
static const float HORIZON_FADE_END = 0.0;

[[vk::binding(0)]]
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_View;
    column_major float4x4 u_Projection;

    float3 u_TopColor;
    float _pad0;
    float3 u_BottomColor;
    float _pad1;
    float u_Time;
    float3 _pad2;
};

struct VertexInput {
    float3 a_Position : POSITION;
};

struct VertexOutput {
    float4 clipPos : SV_Position;
    float3 localPosition : TEXCOORD0;
};

struct FragmentOutput {
    float4 color : SV_Target0;
    int entityID : SV_Target1;
};

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

    output.clipPos = mul(u_Projection, mul(rotView, float4(output.localPosition, 1.0)));
    output.clipPos.z = output.clipPos.w;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output;

    float3 r = normalize(input.localPosition);

    float timeScale = 0.1;
    float wrapRange = TIME_WRAP_MAX - TIME_WRAP_MIN;
    float wrappedTime = fmod(u_Time - TIME_WRAP_MIN, wrapRange) + TIME_WRAP_MIN;
    float3 sunDirection = normalize(float3(sin(timeScale * wrappedTime), cos(timeScale * wrappedTime), 1.0));
    float3 color = skyColor(r, sunDirection);

    // Smoothly fade to black below the horizon.
    float horizonFade = smoothstep(HORIZON_FADE_START, HORIZON_FADE_END, r.y);
    color = lerp(float3(0.0, 0.0, 0.0), color, horizonFade);

    output.color = float4(color, 1.0);
    output.entityID = -1;

    return output;
}

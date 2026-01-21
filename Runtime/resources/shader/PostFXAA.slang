// FXAA Shader - HLSL / Slang

cbuffer FXAAParams : register(b0)
{
    float2 u_InvScreenSize;  // 1.0 / screen resolution (width, height)
}

[[vk::binding(0)]]
Sampler2D u_Input : register(t0);
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
    float2 TexCoord       : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.Position = float4(input.a_Position, 0.0, 1.0);
    output.TexCoord = input.a_TexCoord;
    return output;
}

struct FragmentOutput
{
    float4 color : SV_Target0;
    int entityID : SV_Target1;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput out;

    float2 uv = input.TexCoord;

// Sample luminance at current pixel and neighbors
#define LUMA(texCoord) dot(u_Input.Sample(texCoord).rgb, float3(0.299, 0.587, 0.114))

    float lumaM = LUMA(uv);
    float lumaN = LUMA(uv + float2(0.0, -u_InvScreenSize.y));
    float lumaS = LUMA(uv + float2(0.0,  u_InvScreenSize.y));
    float lumaE = LUMA(uv + float2( u_InvScreenSize.x, 0.0));
    float lumaW = LUMA(uv + float2(-u_InvScreenSize.x, 0.0));

    // Compute edge direction
    float2 dir;
    dir.x = -((lumaN + lumaS) - 2.0 * lumaM);
    dir.y =  ((lumaE + lumaW) - 2.0 * lumaM);

    // Reduce small variations
    float dirReduce = max((lumaN + lumaS + lumaE + lumaW) * 0.25 * 0.5, 1.0/128.0);
    dir = clamp(dir / dirReduce, -8.0, 8.0) * u_InvScreenSize;

    // Sample along the edge
    float4 rgbA = 0.5 * (
        u_Input.Sample(uv + dir * (1.0 / 3.0 - 0.5)) +
                         u_Input.Sample(uv + dir * (2.0/3.0 - 0.5))
    );

    float4 rgbB = rgbA * 0.5 + 0.25 * (
        u_Input.Sample(uv + dir * -0.5) +
        u_Input.Sample(uv + dir * 0.5)
    );

    // Optional: choose between rgbA and rgbB based on luminance contrast
    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaE, lumaW)));

    float4 finalColor = rgbB;
    if (dot(rgbB.rgb, float3(0.299, 0.587, 0.114)) < lumaMin || dot(rgbB.rgb, float3(0.299, 0.587, 0.114)) > lumaMax)
        finalColor = rgbA;

    out.color = finalColor;

    // Propagate entity ID from the bound entity attachment so picking still works
    out.entityID = (int)u_Entity.Sample(uv).r;

    return out;
}

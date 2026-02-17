#include "Library/Types.slang"

cbuffer BlitParams : register(b0)
{
    int u_BlitMode; // 0 = ColorOnly, 1 = ColorAndEntity
    int _pad0;
    int _pad1;
    int _pad2;
};

[[vk::binding(0)]]
Sampler2D u_ColorInput : register(t0);
[[vk::binding(1)]]
ISampler2D u_EntityInput : register(t1);

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
    
    // Copy color from input
    out.color = u_ColorInput.Sample(uv);
    
    // Copy entity ID if in ColorAndEntity mode
    if (u_BlitMode == 1)
    {
        out.entityID = (int)u_EntityInput.Sample(uv).r;
    }
    else
    {
        out.entityID = -1; // No entity
    }
    
    return out;
}

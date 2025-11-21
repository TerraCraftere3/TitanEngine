// RenderQuad.slang
cbuffer GlobalUBO : register(b0)
{
    column_major float4x4 u_ViewProjection;
};

// Use Sampler2D for combined image sampler (better GLSL compatibility)
Sampler2D u_Textures[32];

struct VertexInput
{
    float3 a_Position : POSITION;
    float4 a_Color : COLOR;
    float2 a_UV : TEXCOORD0;
    int a_TexIndex : TEXCOORD1;
    float a_TilingFactor : TEXCOORD2;
    int a_EntityID : TEXCOORD3;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
    nointerpolation int texIndex : TEXCOORD1;
    float tilingFactor : TEXCOORD2;
    nointerpolation int entityID : TEXCOORD3;
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

    int texIndex = input.texIndex;

    // Sample from the appropriate texture slot
    float4 texColor = float4(1.0, 1.0, 1.0, 1.0);

    switch (texIndex)
    {
        case 0: texColor = u_Textures[0].Sample(input.texCoord); break;
        case 1: texColor = u_Textures[1].Sample(input.texCoord); break;
        case 2: texColor = u_Textures[2].Sample(input.texCoord); break;
        case 3: texColor = u_Textures[3].Sample(input.texCoord); break;
        case 4: texColor = u_Textures[4].Sample(input.texCoord); break;
        case 5: texColor = u_Textures[5].Sample(input.texCoord); break;
        case 6: texColor = u_Textures[6].Sample(input.texCoord); break;
        case 7: texColor = u_Textures[7].Sample(input.texCoord); break;
        case 8: texColor = u_Textures[8].Sample(input.texCoord); break;
        case 9: texColor = u_Textures[9].Sample(input.texCoord); break;
        case 10: texColor = u_Textures[10].Sample(input.texCoord); break;
        case 11: texColor = u_Textures[11].Sample(input.texCoord); break;
        case 12: texColor = u_Textures[12].Sample(input.texCoord); break;
        case 13: texColor = u_Textures[13].Sample(input.texCoord); break;
        case 14: texColor = u_Textures[14].Sample(input.texCoord); break;
        case 15: texColor = u_Textures[15].Sample(input.texCoord); break;
        case 16: texColor = u_Textures[16].Sample(input.texCoord); break;
        case 17: texColor = u_Textures[17].Sample(input.texCoord); break;
        case 18: texColor = u_Textures[18].Sample(input.texCoord); break;
        case 19: texColor = u_Textures[19].Sample(input.texCoord); break;
        case 20: texColor = u_Textures[20].Sample(input.texCoord); break;
        case 21: texColor = u_Textures[21].Sample(input.texCoord); break;
        case 22: texColor = u_Textures[22].Sample(input.texCoord); break;
        case 23: texColor = u_Textures[23].Sample(input.texCoord); break;
        case 24: texColor = u_Textures[24].Sample(input.texCoord); break;
        case 25: texColor = u_Textures[25].Sample(input.texCoord); break;
        case 26: texColor = u_Textures[26].Sample(input.texCoord); break;
        case 27: texColor = u_Textures[27].Sample(input.texCoord); break;
        case 28: texColor = u_Textures[28].Sample(input.texCoord); break;
        case 29: texColor = u_Textures[29].Sample(input.texCoord); break;
        case 30: texColor = u_Textures[30].Sample(input.texCoord); break;
        case 31: texColor = u_Textures[31].Sample(input.texCoord); break;
    }

    output.color = texColor * input.color;
    output.entityID = input.entityID;

    return output;
}
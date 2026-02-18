float mix(float a, float b, float t)
{
    return a * (1.0 - t) + b * t;
}

float2 mix(float2 a, float2 b, float t)
{
    return a * (1.0 - t) + b * t;
}

float3 mix(float3 a, float3 b, float t)
{
    return a * (1.0 - t) + b * t;
}

float4 mix(float4 a, float4 b, float t)
{
    return a * (1.0 - t) + b * t;
}

Sampler2D GetBindlessTexture(uint2 handle);

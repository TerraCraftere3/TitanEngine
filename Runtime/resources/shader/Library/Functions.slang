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

float3 wavelengthToRGB(float wavelength)
{
    float3 color = float3(0.0, 0.0, 0.0);

    if (wavelength >= 380.0 && wavelength < 440.0)
    {
        color.r = -(wavelength - 440.0) / (440.0 - 380.0);
        color.g = 0.0;
        color.b = 1.0;
    }
    else if (wavelength >= 440.0 && wavelength < 490.0)
    {
        color.r = 0.0;
        color.g = (wavelength - 440.0) / (490.0 - 440.0);
        color.b = 1.0;
    }
    else if (wavelength >= 490.0 && wavelength < 510.0)
    {
        color.r = 0.0;
        color.g = 1.0;
        color.b = -(wavelength - 510.0) / (510.0 - 490.0);
    }
    else if (wavelength >= 510.0 && wavelength < 580.0)
    {
        color.r = (wavelength - 510.0) / (580.0 - 510.0);
        color.g = 1.0;
        color.b = 0.0;
    }
    else if (wavelength >= 580.0 && wavelength < 645.0)
    {
        color.r = 1.0;
        color.g = -(wavelength - 645.0) / (645.0 - 580.0);
        color.b = 0.0;
    }
    else if (wavelength >= 645.0 && wavelength <= 780.0)
    {
        color.r = 1.0;
        color.g = 0.0;
        color.b = 0.0;
    }

    // Intensity correction
    float intensity = 1.5;                          // Adjust this value for brightness
    return pow(color * intensity, float3(1 / 2.2)); // Gamma correction
}

Sampler2D GetBindlessTexture(uint2 handle);

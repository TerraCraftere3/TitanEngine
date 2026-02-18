#include "../Library/Constants.slang"
#include "../Library/Functions.slang"
#include "BGConstants.slang"

float2x2 mm2(in float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2x2(c, s, -s, c);
}

float raySphereIntersect(float3 rayOrigin, float3 rayDirection, float3 sphereCenter, float sphereRadius)
{
    float a = dot(rayDirection, rayDirection);
    float3 d = rayOrigin - sphereCenter;
    float b = 2.0 * dot(rayDirection, d);
    float c = dot(d, d) - (sphereRadius * sphereRadius);
    if (b * b - 4.0 * a * c < 0.0)
    {
        return -1.0;
    }
    return (-b + sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
}

float rayleighPhase(float mu)
{
    float phase = (3.0 / (16.0 * PI)) * (1.0 + mu * mu);
    return phase;
}

// The mie phase function
float miePhase(float mu)
{
    float numerator = (1.0 - G * G) * (1.0 + mu * mu);
    float denominator = (2.0 + G * G) * pow(1.0 + G * G - 2.0 * G * mu, 3.0 / 2.0);
    return (3.0 / (8.0 * PI)) * numerator / denominator;
}

// Returns the expected amount of atmospheric scattering at a given height above sea level
// Different parameters are passed in for rayleigh and mie scattering
float3 scatteringAtHeight(float3 scatteringAtSea, float height, float heightScale)
{
    return scatteringAtSea * exp(-height / heightScale);
}

// Returns the height of a vector above the 'earth'
float height(float3 p)
{
    return (length(p) - EARTHRADIUS);
}

// Calculates the transmittance from pb to pa, given the scale height and the scattering
// coefficients. The samples parameter controls how accurate the result is.
// See the scratchapixel link for details on what is happening
float3 transmittance(float3 pa, float3 pb, int samples, float scaleHeight, float3 scatCoeffs)
{
    float opticalDepth = 0.0;
    float segmentLength = length(pb - pa) / float(samples);
    for (int i = 0; i < samples; i++)
    {
        float3 samplePoint = mix(pa, pb, (float(i) + 0.5) / float(samples));
        float sampleHeight = height(samplePoint);
        opticalDepth += exp(-sampleHeight / scaleHeight) * segmentLength;
    }
    float3 transmittance = exp(-1.0 * scatCoeffs * opticalDepth);
    return transmittance;
}

// This is the main function that uses the ideas of rayleigh and mie scattering
// This function is written with understandability in mind rather than performance, and
// redundant calls to transmittance can be removed as per the code in the scratchapixel link

float3 getSkyBackgroundColor(float3 pa, float3 pb, float3 sunDir)
{
    // Get the angle between the ray direction and the sun
    float mu = dot(normalize(pb - pa), sunDir);

    // Calculate the result from the phase functions
    float phaseR = rayleighPhase(mu);
    float phaseM = miePhase(mu);

    // Will be used to store the cumulative colors for rayleigh and mie
    float3 rayleighColor = float3(0.0, 0.0, 0.0);
    float3 mieColor = float3(0.0, 0.0, 0.0);

    // Performs an integral approximation by checking a number of sample points and:
    //		- Calculating the incident light on that point from the sun
    //		- Calculating the amount of that light that gets reflected towards the origin

    int samples = 10;
    float segmentLength = length(pb - pa) / float(samples);

    for (int i = 0; i < samples; i++)
    {
        float3 samplePoint = mix(pa, pb, (float(i) + 0.5) / float(samples));
        float sampleHeight = height(samplePoint);
        float distanceToAtmosphere = raySphereIntersect(samplePoint, sunDir, float3(0.0, 0.0, 0.0), ATMOSPHERERADIUS);
        float3 atmosphereIntersect = samplePoint + sunDir * distanceToAtmosphere;

        // Rayleigh Calculations
        float3 trans1R = transmittance(pa, samplePoint, 10, RAYLEIGHSCALEHEIGHT, BETAR);
        float3 trans2R = transmittance(samplePoint, atmosphereIntersect, 10, RAYLEIGHSCALEHEIGHT, BETAR);
        rayleighColor +=
            trans1R * trans2R * scatteringAtHeight(BETAR, sampleHeight, RAYLEIGHSCALEHEIGHT) * segmentLength;

        // Mie Calculations
        float3 trans1M = transmittance(pa, samplePoint, 10, MIESCALEHEIGHT, BETAM);
        float3 trans2M = transmittance(samplePoint, atmosphereIntersect, 10, MIESCALEHEIGHT, BETAM);
        mieColor += trans1M * trans2M * scatteringAtHeight(BETAM, sampleHeight, MIESCALEHEIGHT) * segmentLength;
    }

    rayleighColor = SUNINTENSITY * phaseR * rayleighColor;
    mieColor = SUNINTENSITY * phaseM * mieColor;

    return rayleighColor + mieColor;
}

// Get the sky color for the ray in direction 'p'
float3 skyBackgroundColor(float3 p, float3 sunDir)
{
    // Get the origin and direction of the ray
    float3 origin = float3(0.0, EARTHRADIUS + 1.0, 0.0);
    float3 dir = p;

    // Get the position where the ray 'leaves' the atmopshere (see the scratchapixel link for details)
    // Note that this implementation only works when the origin is inside the atmosphere to begin with
    float distanceToAtmosphere = raySphereIntersect(origin, dir, float3(0.0, 0.0, 0.0), ATMOSPHERERADIUS);
    float3 atmosphereIntersect = origin + dir * distanceToAtmosphere;

    // Get the color of the light from the origin to the atmosphere intersect
    float3 col = getSkyBackgroundColor(origin, atmosphereIntersect, sunDir);
    return col;
}
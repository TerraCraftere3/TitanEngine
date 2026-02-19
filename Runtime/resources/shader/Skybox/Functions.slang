#include "BGFunctions.slang"

static const float TIME_WRAP_MIN = -17.0;
static const float TIME_WRAP_MAX = 17.0;
static const float HORIZON_FADE_START = -0.1;
static const float HORIZON_FADE_END = 0.0;

float3 skyColor(float3 rayDirection, float time)
{
    float timeScale = 0.1;
    float wrapRange = TIME_WRAP_MAX - TIME_WRAP_MIN;
    float wrappedTime = fmod(time - TIME_WRAP_MIN, wrapRange) + TIME_WRAP_MIN;
    float3 sunDirection = normalize(float3(sin(timeScale * wrappedTime), cos(timeScale * wrappedTime), 1.0));
    float3 color = skyBackgroundColor(rayDirection, sunDirection);

    float horizonFade = smoothstep(HORIZON_FADE_START, HORIZON_FADE_END, rayDirection.y);
    float3 groundColor = float3(0.05, 0.05, 0.05);
    color = lerp(groundColor, color, horizonFade);

    // Force gray below the horizon
    if (rayDirection.y < HORIZON_FADE_START)
    {
        color = groundColor;
    }

    return color;
}
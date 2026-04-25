#include "fbmNoise.h"

FbmNoise::FbmNoise(int seed, int octaveCount, float lacunarity, float persistence, InterpolateFunc interpolate) : perlin(seed, interpolate)
{
    this->octaveCount = octaveCount;
    this->lacunarity = lacunarity;
    this->persistence = persistence;
}

FbmNoise::~FbmNoise() = default;

float FbmNoise::sample(float fx, float fy)
{
    float value = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < octaveCount; i++) {
        value += perlin.sample(fx * frequency, fy * frequency) * amplitude;

        frequency *= lacunarity;
        amplitude /= persistence;
    }

    // "Contrast"
    value *= 1.2f;

    // Clamp values, since they can go beyond 1 / -1.
    if (value > 1.0f) {
        value = 1.0f;
    }
    else if (value < -1.0f) {
        value = -1.0f;
    }

    // remap from [-1, 1] to [0, 1]
    value = value * 0.5f + 0.5f;

    return value;
}

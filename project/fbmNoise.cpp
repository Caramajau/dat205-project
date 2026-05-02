#include "fbmNoise.h"

// Using gradients to create an erosion look idea from: https://www.youtube.com/watch?v=gsJHzBTPG0Y

FbmNoise::FbmNoise(int seed, int octaveCount, float lacunarity, float persistence, InterpolateFunc interpolate, InterpolateFunc interpolateDerivative, ErosionFunc erosion, float erosionStrength) 
    : perlin(seed, interpolate, interpolateDerivative)
{
    this->octaveCount = octaveCount;
    this->lacunarity = lacunarity;
    this->persistence = persistence;
    this->erosion = erosion;
    this->erosionStrength = erosionStrength;
}

FbmNoise::~FbmNoise() = default;

float FbmNoise::sample(float fx, float fy) const
{
    float value = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    glm::vec2 d(0.0f);

    for (int i = 0; i < octaveCount; i++) {
        float dx = 0.0f;
        float dy = 0.0f;
        float current = perlin.sample(fx * frequency, fy * frequency, dx, dy);

        dx *= frequency;
        dy *= frequency;

        // Erosion weight to suppress detail on steep slopes
        // i.e bigger gradient, less contribution
        float length = glm::dot(d, d);
        float weight = erosion(length, erosionStrength);

        value += current * amplitude * weight;

        // Accumulate the derivative after so that gradients affect all octaves after the first
        d.x += amplitude * dx;
        d.y += amplitude * dy;

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

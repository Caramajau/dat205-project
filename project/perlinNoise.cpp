#include "perlinNoise.h"

// Perlin noise implementation (Fbm part also) heavily based on
// https://www.youtube.com/watch?v=kCIaHqb60Cw
// For instance, one difference is that this uses glm at various places.

PerlinNoise::PerlinNoise(int seed, InterpolateFunc interpolate, InterpolateFunc interpolateDerivative, bool useIncorrectBlending) {
    this->seed = seed;
    this->interpolate = interpolate;
    this->interpolateDerivative = interpolateDerivative;
    this->useIncorrectBlending = useIncorrectBlending;
}

PerlinNoise::~PerlinNoise() = default;

// Sample Perlin noise at coordinates x, y
float PerlinNoise::sample(float x, float y, float& outDx, float& outDy) const {
    // Determine grid cell corner coordinates
    auto x0 = (int)x;
    auto y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Compute interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Compute and interpolate top two corners
    float topLeftDot = dotGridGradient(x0, y0, x, y);
    float topRightDot = dotGridGradient(x1, y0, x, y);
    float topInterpolation = interpolate(sx);
    float topBlending = useIncorrectBlending ?
        incorrectBlending(topLeftDot, topRightDot, topInterpolation) : blending(topLeftDot, topRightDot, topInterpolation);

    // Compute and interpolate bottom two corners
    float bottomLeftDot = dotGridGradient(x0, y1, x, y);
    float bottomRightDot = dotGridGradient(x1, y1, x, y);
    float bottomInterpolation = interpolate(sx);
    float bottomBlending = useIncorrectBlending ?
        incorrectBlending(bottomLeftDot, bottomRightDot, bottomInterpolation) : blending(bottomLeftDot, bottomRightDot, bottomInterpolation);

    // Then interpolate horizontal with vertical
    // I.e.: interpolate between the two previously interpolated values, now in y.
    float finalInterpolation = interpolate(sy);
    float finalBlending = useIncorrectBlending ?
        incorrectBlending(bottomLeftDot, bottomRightDot, bottomInterpolation) : blending(topBlending, bottomBlending, finalInterpolation);

    // Derivative of chosen interpolation
    float dx = interpolateDerivative(sx);
    float dy = interpolateDerivative(sy);

    // Account for chain rule
    float topContribution = topRightDot - topLeftDot;
    float bottomContribution = finalInterpolation * (bottomRightDot - bottomLeftDot - topRightDot + topLeftDot);
    outDx = dx * (topContribution + bottomContribution);
    outDy = dy * (bottomBlending - topBlending);

    return finalBlending;
}

// Computes the dot product of the distance and gradient vectors
float PerlinNoise::dotGridGradient(int integerX, int integerY, float x, float y) const {
    // Get gradient from integer coordinates, i.e. gradient from a grid corner
    glm::vec2 gradient = randomGradient(integerX, integerY);

    // Compute the distance vector to the grid corner
    float dx = x - (float)integerX;
    float dy = y - (float)integerY;
    auto distanceVector = glm::vec2(dx, dy);

    // Compute the dot-product.
    return glm::dot(distanceVector, gradient);
}

// Want random values to be repeatable, uses this hashing function, which is pseudo-random.
glm::vec2 PerlinNoise::randomGradient(int integerX, int integerY) const {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned int w = 8 * sizeof(unsigned int);
    const unsigned int s = w / 2;

    // Two big arbitrary numbers for seed implementation,
    // so that negative seeds aren't just flipped positive ones.
    unsigned int a = integerX ^ (seed * 1234567891u);
    unsigned int b = integerY ^ (seed * 3214567891u);
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;

    // Normalise to range [0, 2*Pi]
    float random = a * (3.14159265 / ~(~0u >> 1));

    // Create the vector from the angle
    auto v = glm::vec2(sin(random), cos(random));

    return v;
}

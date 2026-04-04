#include "perlin.h"

// Perlin noise implementation
// https://www.youtube.com/watch?v=kCIaHqb60Cw
// NOTE: did change some things to use glm.

// Sample Perlin noise at coordinates x, y
float perlin(float x, float y) {
	// Determine grid cell corner coordinates
	auto x0 = (int)x;
	auto y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Compute interpolation weights
	float sx = x - (float)x0;
	float sy = y - (float)y0;

    // Compute and interpolate top two corners
    float topLeftDotGradient = dotGridGradient(x0, y0, x, y);
    float topRightDotGradient = dotGridGradient(x1, y0, x, y);
    float horizontalTopInterpolation = interpolate(
        topLeftDotGradient,
        topRightDotGradient,
        sx
    );

    // Compute and interpolate bottom two corners
    float bottomLeftDotGradient = dotGridGradient(x0, y1, x, y);
    float bottomRightDotGradient = dotGridGradient(x1, y1, x, y);
    float horizontalBottomInterpolation = interpolate(
        bottomLeftDotGradient,
        bottomRightDotGradient,
        sx
    );

    // Then interpolate horizontal with vertical
    // I.e.: interpolate between the two previously interpolated values, now in y.
    float finalInterpolation = interpolate(
        horizontalTopInterpolation, 
        horizontalBottomInterpolation, 
        sy
    );

    return finalInterpolation;
}

// Computes the dot product of the distance and gradient vectors
float dotGridGradient(int integerX, int integerY, float x, float y) {
	// Get gradient from integer coordinates
    vec2 gradient = randomGradient(integerX, integerY);

    // Compute the distance vector
    float dx = x - (float)integerX;
    float dy = y - (float)integerY;
    auto distanceVector = vec2(dx, dy);

    // Compute the dot-product.
    return dot(distanceVector, gradient);
}

// Want random values to be repeatable, uses this hashing function, which is pseudo-random.
vec2 randomGradient(int integerX, int integerY) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = integerX;
    unsigned b = integerY;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    auto v = vec2(sin(random), cos(random));

    return v;
}

// Will interpolate between the first and second value, weight between 0 and 1.
// NOTE: This function is instead based on the formula from this video:
// https://www.youtube.com/watch?v=ZsEnnB2wrbI
float interpolate(float a, float b, float weight) {
    float t = weight * weight * weight * (weight * (weight * 6.0f - 15.0f) + 10.0f);
    return a + (b - a) * t;
}

#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include "interpolations.h"

class PerlinNoise {
public:
	PerlinNoise(int seed, InterpolateFunc interpolate, InterpolateFunc interpolateDerivative);
	~PerlinNoise();

	float sample(float x, float y, float& outDx, float& outDy) const;

private:
	int seed;
	InterpolateFunc interpolate;
	InterpolateFunc interpolateDerivative;

	float dotGridGradient(int integerX, int integerY, float x, float y) const;
	glm::vec2 randomGradient(int integerX, int integerY) const;
};

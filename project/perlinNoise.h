#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include "interpolations.h"

class PerlinNoise {
public:
	PerlinNoise(int seed, SmoothFunc smooth, SmoothFunc smoothDerivative, bool useIncorrectLerp);
	~PerlinNoise();

	float sample(float x, float y, float& outDx, float& outDy) const;

private:
	int seed;
	SmoothFunc smooth;
	SmoothFunc smoothDerivative;
	bool useIncorrectLerp;

	float dotGridGradient(int integerX, int integerY, float x, float y) const;
	glm::vec2 randomGradient(int integerX, int integerY) const;
};

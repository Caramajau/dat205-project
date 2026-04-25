#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include "interpolations.h"

class FbmNoise {
public:
	FbmNoise(int seed, int octaveCount, float lacunarity, float persistence, InterpolateFunc interpolate);
	~FbmNoise();
	float sample(float fx, float fy);

private:
	int seed;
	int octaveCount;
	float lacunarity;
	float persistence;
	InterpolateFunc interpolate;

	// TODO: Move?
	float perlin(float x, float y);
	float dotGridGradient(int integerX, int integerY, float x, float y) const;
	glm::vec2 randomGradient(int integerX, int integerY) const;
};

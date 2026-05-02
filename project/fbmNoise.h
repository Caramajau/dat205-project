#pragma once

#include <glm/glm.hpp>
#include "interpolations.h"
#include "perlinNoise.h"
#include "erosion.h"

class FbmNoise {
public:
	FbmNoise(int seed, int octaveCount, float lacunarity, float persistence, InterpolateFunc interpolate, InterpolateFunc interpolateDerivative, ErosionFunc erosion, float erosionStrength);
	~FbmNoise();
	float sample(float fx, float fy) const;

private:
	int octaveCount;
	float lacunarity;
	float persistence;
	ErosionFunc erosion;
	float erosionStrength;

	PerlinNoise perlin;
};

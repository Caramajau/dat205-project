#pragma once

#include <glm/glm.hpp>
#include "interpolations.h"
#include "perlinNoise.h"

class FbmNoise {
public:
	FbmNoise(int seed, int octaveCount, float lacunarity, float persistence, InterpolateFunc interpolate);
	~FbmNoise();
	float sample(float fx, float fy);

private:
	int octaveCount;
	float lacunarity;
	float persistence;

	PerlinNoise perlin;
};

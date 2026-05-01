#pragma once

#include <vector>
#include "interpolations.h"
#include "fbmNoise.h"

std::vector<float> createHeightMap(int seed, int width, int height, int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType, int warpLevel);

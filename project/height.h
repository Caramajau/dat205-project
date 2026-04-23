#pragma once

#include <vector>
#include "interpolations.h"
#include "fbmNoise.h"

std::vector<float> createHeightMap(int seed, int width, int height, int gridSize, int octaveCount = 8, float lacunarity = 2.0f, float persistence = 2.0f, InterpolationType interpolationType = InterpolationType::Quintic);

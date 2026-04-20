#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <vector>
#include "interpolations.h"

std::vector<float> createPerlinGrid(int seed, int width, int height, int gridSize, int octaveCount = 8, float lacunarity = 2.0f, float persistence = 2.0f, InterpolationType interpolationType = InterpolationType::Quintic);

float perlin(int seed, float x, float y, InterpolateFunc interpolate);

float dotGridGradient(int seed, int integerX, int integerY, float x, float y);

glm::vec2 randomGradient(int seed, int integerX, int integerY);

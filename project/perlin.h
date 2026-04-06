#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <vector>
#include "interpolations.h"

using namespace glm;

std::vector<float> createPerlinGrid(int width, int height, int gridSize, float lacunarity = 2.0f, float persistence = 2.0f, InterpolationType interpolationType = InterpolationType::Quintic);

float perlin(float x, float y, InterpolateFunc interpolate);

float dotGridGradient(int integerX, int integerY, float x, float y);

vec2 randomGradient(int integerX, int integerY);

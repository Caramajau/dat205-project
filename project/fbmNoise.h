#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include "interpolations.h"

float fbm(int octaveCount, int seed, float fx, float fy, InterpolateFunc interpolate, float lacunarity, float persistence);

float perlin(int seed, float x, float y, InterpolateFunc interpolate);

float dotGridGradient(int seed, int integerX, int integerY, float x, float y);

glm::vec2 randomGradient(int seed, int integerX, int integerY);

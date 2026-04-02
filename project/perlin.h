#pragma once

#include <glm/glm.hpp>
#include <math.h>

using namespace glm;

float perlin(float x, float y);

float dotGridGradient(int integerX, int integerY, float x, float y);

vec2 randomGradient(int integerX, int integerY);

float interpolate(float a, float b, float weight);

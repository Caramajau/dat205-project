#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <vector>

using namespace glm;

std::vector<float> createPerlinGrid(int width, int height, int gridSize);

float perlin(float x, float y);

float dotGridGradient(int integerX, int integerY, float x, float y);

vec2 randomGradient(int integerX, int integerY);

float interpolate(float a, float b, float weight);

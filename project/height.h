#pragma once

#include <vector>
#include "interpolations.h"
#include "fbmNoise.h"
#include "erosion.h"
#include "ProceduralConfig.h"

std::vector<float> createHeightMap(const ProceduralConfig& config);

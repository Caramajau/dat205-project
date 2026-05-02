#pragma once
#include "interpolations.h"
#include "erosion.h"

struct ProceduralConfig {
	int seed = 0;

	int width = 256;
	int height = 256;
	int gridSize = 256;

	int octaveCount = 8;
	float lacunarity = 2.0f;
	float persistence = 2.0f;

	InterpolationType interpolationType = InterpolationType::Quintic;

	ErosionType erosionType = ErosionType::Rational;
	float erosionStrength = 1.0f;

	int warpLevel = 2;

	// NOTE: only used by terrain and not display
	float heightScale = 100.0f;

	void reset() { *this = ProceduralConfig{}; }
};

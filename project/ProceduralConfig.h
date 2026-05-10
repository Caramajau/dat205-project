#pragma once

#include <glm/glm.hpp>
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

	// Configs used only by terrain and not display
	float heightScale = 100.0f;
	bool useNeighbours = true;
	glm::vec3 sunDirection = glm::vec3(0.8, 1.0, 0.6);

	void reset() { *this = ProceduralConfig{}; }
};

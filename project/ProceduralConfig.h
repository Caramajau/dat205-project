#pragma once

#include <glm/glm.hpp>
#include "interpolations.h"
#include "erosion.h"

struct ProceduralConfig {
	int seed = 0;

	int width = 256;
	int length = 256;
	int gridSize = 256;

	int octaveCount = 8;
	float lacunarity = 2.0f;
	float persistence = 0.5f;

	InterpolationType interpolationType = InterpolationType::Quintic;
	bool useIncorrectBlending = false;

	ErosionType erosionType = ErosionType::Rational;
	float erosionStrength = 1.0f;

	int warpLevel = 2;
	float warpAmplitude = 4.0f;

	// Configs used only by terrain and not display
	float terrainLevel = -100.0f;
	float heightScale = 100.0f;
	bool useNeighbours = true;
	glm::vec3 sunDirection = glm::vec3(0.8, 1.0, 0.6);

	float textureZoom = 0.1f;
	float grassThreshold = 0.2f;
	float rockThreshold = 0.4f;
	float sandThreshold = 1.0f;
	float sandLevelOffset = 0.5f;
	float triplanarBlendFactor = 4.0f;

	// Water configs
	float waterLevel = -58.0f;
	float waterTiling = 0.04f;

	float waterWaveSpeed = 0.03f;
	float waterWaveStrength = 0.04f;

	float waterShineDamper = 20.0f;
	float waterReflectivity = 0.5f;

	float waterBorderTransparencyFactor = 5.0f;
	float waterDistortionDampening = 20.0f;
	float waterHighlightDampening = 5.0f;

	float waterFresnelModifier = 2.0f;

	float waterNormalFlattenFactor = 3.0f;

	float waterMurkyColourFactor = 60.0f;
	float waterBlueTintFactor = 0.05f;

	void reset() { *this = ProceduralConfig{}; }
};

#pragma once

#include <vector>
#include <random>

// Implementation heavily based on: https://github.com/SebLague/Hydraulic-Erosion/blob/master/Assets/Scripts/Erosion.cs

struct HeightAndGradient {
	float height;
	float gradientX;
	float gradientY;
};

class Erosion {
public:
	void erode(std::vector<float> map, int mapSize, int numInterations = 1, bool resetSeed = false);
private:
	int seed = 0;

	int erosionRadius = 3;

	// At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
	float inertia = 0.05f;

	float sedimentCapacityFactor = 4;
	float minSedimentCapacity = 0.01f;

	float erodeSpeed = 0.3f;
	float depositSpeed = 0.3f;
	float evaporateSpeed = 0.01f;

	float gravity = 4;

	int maxDropletLifetime = 30;

	float initialWaterVolume = 1;
	float initialSpeed = 1;

	std::vector<std::vector<int>> erosionBrushIndices;
	std::vector<std::vector<float>> erosionBrushWeights;
	std::mt19937 prng;

	int currentSeed;
	int currentErosionRadius;
	int currentMapSize;

	void initialize(int mapSize, bool resetSeed);
	HeightAndGradient calculateHeightAndGradient(std::vector<float> nodes, int mapSize, float posX, float posY);
	void initializeBrushIndices(int mapSize, int radius);
};

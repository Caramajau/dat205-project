#include "erosion.h"

void Erosion::initialize(int mapSize, bool resetSeed)
{
	// Skipped prng null check
	if (resetSeed || currentSeed != seed) {
		prng.seed(seed);
		currentSeed = seed;
	}

	// Skipped erosionBrushIndices null check
	if (currentErosionRadius != erosionRadius || currentMapSize != mapSize) {
		initializeBrushIndices(mapSize, erosionRadius);
		currentErosionRadius = erosionRadius;
		currentMapSize = mapSize;
	}
}

void Erosion::erode(std::vector<float> map, int mapSize, int numInterations, bool resetSeed)
{
	initialize(mapSize, resetSeed);

	std::uniform_int_distribution<int> dist(0, mapSize - 2);

	for (int iteration = 0; iteration < numInterations; iteration++) {
		float posX = dist(prng);
		float posY = dist(prng);

		float dirX = 0;
		float dirY = 0;

		float speed = initialSpeed;
		float water = initialWaterVolume;
		float sediment = 0;

		for (int lifetime = 0; lifetime < maxDropletLifetime; lifetime++) {
			int nodeX = posX;
			int nodeY = posY;

			int dropletIndex = nodeY * mapSize + nodeX;

			// Calculate droplet's offset inside the cell (0, 0) == at NorthWest node, (1, 1) == at SouthEast node
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posY - nodeY;

			// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
			HeightAndGradient heightAndGradient = calculateHeightAndGradient(map, mapSize, posX, posY);

			// Update the droplet's direction and position (move position 1 unit regardless of speed)
			dirX = (dirX * inertia - heightAndGradient.gradientX * (1 - inertia));
			dirY = (dirY * inertia - heightAndGradient.gradientY * (1 - inertia));

			// Normalise direction
			float len = sqrtf(dirX * dirX + dirY * dirY);
			if (len != 0) {
				dirX /= len;
				dirY /= len;
			}

			posX += dirX;
			posY += dirY;

			// Stop simulating droplet if it's not moving or has flowed over edge of map
			if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= mapSize - 1 || posY < 0 || posY >= mapSize - 1) {
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = calculateHeightAndGradient(map, mapSize, posX, posY).height;
			float deltaHeight = newHeight - heightAndGradient.height;

			// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
			float sedimentCapacity = std::max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);

			// If carrying more sediment than capacity, or if flowing uphill:
			if (sediment > sedimentCapacity || deltaHeight > 0) {
				// If moving uphill (deltaHeight > 0) try fill up to the current height, 
				// otherwise deposit a fraction of the excess sediment
				float amountToDeposit = (deltaHeight > 0) ? std::min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
				sediment -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				map[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				map[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				map[dropletIndex + mapSize] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				map[dropletIndex + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;

			} else {
				// Erode a fraction of the droplet's current carry capacity
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndices[dropletIndex].size(); brushPointIndex++) {
					int nodeIndex = erosionBrushIndices[dropletIndex][brushPointIndex];
					float weightedErodeAmount = amountToErode * erosionBrushWeights[dropletIndex][brushPointIndex];
					float deltaSediment = (map[nodeIndex] < weightedErodeAmount) ? map[nodeIndex] : weightedErodeAmount;
					map[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;
				}

				// Update droplet's speed and water content
				speed = sqrtf(speed * speed + deltaHeight * gravity);
				water *= (1 - evaporateSpeed);
			}
		}

	}
}

HeightAndGradient Erosion::calculateHeightAndGradient(std::vector<float> nodes, int mapSize, float posX, float posY)
{
	int coordX = posX;
	int coordY = posY;

	// Calculate droplet's offset inside the cell (0, 0) at NorthWest node, (1, 1) at SouthEast node
	float x = posX - coordX;
	float y = posY - coordY;

	// Calculate heights of the four nodes of droplet's cell
	int nodeIndexNW = coordY * mapSize + coordX;
	float heightNW = nodes[nodeIndexNW];
	float heightNE = nodes[nodeIndexNW + 1];
	float heightSW = nodes[nodeIndexNW + mapSize];
	float heightSE = nodes[nodeIndexNW + mapSize + 1];

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

	return { height, gradientX, gradientY };
}

void Erosion::initializeBrushIndices(int mapSize, int radius)
{
	erosionBrushIndices.resize(mapSize * mapSize);
	erosionBrushWeights.resize(mapSize * mapSize);

	std::vector<int> xOffsets(radius * radius * 4);
	std::vector<int> yOffsets(radius * radius * 4);
	std::vector<float> weights(radius * radius * 4);
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < erosionBrushIndices.size(); i++) {
		int centreX = i % mapSize;
		int centreY = i / mapSize;

		if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius) {
			weightSum = 0;
			addIndex = 0;

			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					float squareDistance = x * x + y * y;

					if (squareDistance < radius * radius) {
						int coordX = centreX + x;
						int coordY = centreY + y;

						if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {
							float weight = 1 - sqrtf(squareDistance) / radius;
							weightSum += weight;
							weights[addIndex] = weight;
							xOffsets[addIndex] = x;
							yOffsets[addIndex] = y;
							addIndex++;
						}
					}
				}
			}
		}

		int numEntries = addIndex;
		erosionBrushIndices[i].resize(numEntries);
		erosionBrushWeights[i].resize(numEntries);
		
		for (int j = 0; j < numEntries; j++) {
			erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
			erosionBrushWeights[i][j] = weights[j] / weightSum;
		}
	}
}

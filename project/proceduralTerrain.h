#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "height.h"
#include "interpolations.h"
#include "ProceduralConfig.h"

class ProceduralTerrain {
public:
	const float yOffset = -100.0f;

	explicit ProceduralTerrain();
	~ProceduralTerrain();

	void loadShader(bool is_reload);
	void loadTerrainTexture(GLuint& texture, const char* filepath) const;
	void setGpuData(const ProceduralConfig& config);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const;

	const std::vector<float>& getHeightMapGrid() const { return heightMapGrid; }

private:
	// NOTE: If world up is changed from 0, 1, 0 this should match.
	// (probably won't in this project)
	const glm::mat4 terrainModelMatrix = translate(yOffset * glm::vec3(0.0f, 1.0f, 0.0f));

	GLuint perlinTexture = 0;

	GLuint grassTexture = 0;
	GLuint grassNormalMap = 0;
	GLuint rockTexture = 0;
	GLuint rockNormalMap = 0;

	GLuint terrainShader = 0;

	GLuint terrainVertexArrayObject = 0;
	GLuint terrainVertexBufferObject = 0;
	GLuint terrainIndexBufferObject = 0;

	std::vector<float> heightMapGrid;

	size_t triangleCount = 0;

	float heightScale = 0;
	int gridSize = 0;
	bool useNeighbours = false;
	glm::vec3 sunDirection;

	std::vector<float> createVertices(int width, int height) const;
	std::vector<unsigned int> createIndices(int width, int height) const;
};

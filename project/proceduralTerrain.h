#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "perlin.h"
#include "interpolations.h"

class ProceduralTerrain {
public:
	explicit ProceduralTerrain();
	~ProceduralTerrain();

	void loadShader(bool is_reload);
	void setGpuData(int perlinWidth, int perlinHeight, int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType, float initialHeightScale);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const;

private:
	// NOTE: If world up is changed from 0, 1, 0 this should match.
	// (probably won't in this project)
	glm::mat4 terrainModelMatrix = translate(-100.0f * glm::vec3(0.0f, 1.0f, 0.0f));

	GLuint perlinTexture = 0;
	GLuint terrainShader = 0;

	GLuint terrainVertexArrayObject = 0;
	GLuint terrainVertexBufferObject = 0;
	GLuint terrainIndexBufferObject = 0;

	std::vector<float> heightMapGrid;

	size_t triangleCount = 0;

	float heightScale = 0;

	std::vector<float> createVertices(int width, int height) const;
	std::vector<unsigned int> createIndices(int width, int height) const;
};

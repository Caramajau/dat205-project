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
	void initGpuData(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const;
	void reloadTexture(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType);

private:
	// NOTE: If world up is changed from 0, 1, 0 this should match.
	// (probably won't in this project)
	glm::mat4 terrainModelMatrix = translate(100.0f * glm::vec3(0.0f, 1.0f, 0.0f));

	GLuint perlinTexture = 0;
	GLuint terrainShader = 0;

	GLuint terrainVertexArrayObject = 0;
	GLuint terrainVertexBufferObject = 0;
	GLuint terrainIndexBufferObject = 0;

	std::vector<float> heightMapGrid;

	int perlinWidth = 256;
	int perlinHeight = 256;
};

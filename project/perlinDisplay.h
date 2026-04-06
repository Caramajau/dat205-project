#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "perlin.h"

using namespace glm;

class PerlinDisplay {
public:
	explicit PerlinDisplay();
	~PerlinDisplay();

	void loadShader(bool is_reload);
	void initGpuData(float lacunarity, float persistence, int gridSize);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const;
	void reloadTexture(float lacunarity, float persistence, int gridSize);

private:
	// NOTE: If world up is changed from 0, 1, 0 this should match.
	// (probably won't in this project)
	mat4 perlinNoiseModelMatrix = translate(100.0f * vec3(0.0f, 1.0f, 0.0f));

	GLuint perlinTexture = 0;
	GLuint perlinShader = 0;

	GLuint quadVertexArrayObject = 0;
	GLuint quadVertexBufferObject = 0;
	GLuint quadIndexBufferObject = 0;

	std::vector<float> grid;

	int perlinWidth = 1000;
	int perlinHeight = 1000;
};

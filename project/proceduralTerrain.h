#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "height.h"
#include "interpolations.h"
#include "ProceduralConfig.h"
#include "textureUtils.h"

class ProceduralTerrain {
public:
	explicit ProceduralTerrain();
	~ProceduralTerrain();

	void loadShader(bool is_reload, bool useLowResTextures);
	void setGpuData(const ProceduralConfig& config, const std::vector<float>& heightMapGrid);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec4& waterPlane, const glm::vec3& lightPosition, const ProceduralConfig& config);

private:
	glm::mat4 terrainModelMatrix;

	GLuint perlinTexture = 0;

	GLuint grassTexture = 0;
	GLuint grassNormalMap = 0;

	GLuint rockTexture = 0;
	GLuint rockNormalMap = 0;

	GLuint sandTexture = 0;
	GLuint sandNormalMap = 0;

	GLuint snowTexture = 0;
	GLuint snowNormalMap = 0;

	GLuint terrainShader = 0;

	GLuint terrainVertexArrayObject = 0;
	GLuint terrainVertexBufferObject = 0;
	GLuint terrainIndexBufferObject = 0;

	size_t triangleCount = 0;

	std::vector<float> createVertices(int width, int length) const;
	std::vector<unsigned int> createIndices(int width, int length) const;
};

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

	void loadShader(bool is_reload);
	void setGpuData(const ProceduralConfig& config);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec4& waterPlane) const;

	const std::vector<float>& getHeightMapGrid() const { return heightMapGrid; }

	float getLevel() const;
	void setLevel(float newLevel);

private:
	float terrainLevel;
	glm::mat4 terrainModelMatrix;

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
	bool useNeighbours = false;
	glm::vec3 sunDirection;

	float textureZoom;
	float grassThreshold;
	float rockThreshold;
	float triplanarBlendFactor;

	std::vector<float> createVertices(int width, int length) const;
	std::vector<unsigned int> createIndices(int width, int length) const;
};

#pragma once

#include "ProceduralConfig.h"
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "waterFrameBuffers.h"
#include <stb_image.h>

class Water {
public:
	void loadShader(bool is_reload);
	void setGpuData(const ProceduralConfig& config, const WaterFrameBuffers& waterFBOs);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, float deltaTime);

private:
	// TODO make height customisable?
	glm::mat4 waterModelMatrix = translate(-80.0f * glm::vec3(0.0f, 1.0f, 0.0f));

	GLuint waterShader = 0;

	GLuint waterVertexArrayObject = 0;
	GLuint waterVertexBufferObject = 0;
	GLuint waterIndexBufferObject = 0;

	GLuint reflectionTexture = 0;
	GLuint refractionTexture = 0;

	GLuint dudvTexture = 0;

	// TODO: make customsiable?
	float waveSpeed = 0.03f;
	float moveFactor = 0.0f;

	void loadDuDvTexture(GLuint& texture, const char* filepath) const;
};

#pragma once

#include "ProceduralConfig.h"
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <labhelper.h>
#include "waterFrameBuffers.h"
#include "textureUtils.h"

class Water {
public:
	void loadShader(bool is_reload);
	void setGpuData(const ProceduralConfig& config, const WaterFrameBuffers& waterFBOs);
	void submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, float deltaTime, const glm::vec3& cameraPosition, float near, float far, glm::vec3 lightPosition);
	
	float getLevel() const;

private:
	float level;
	glm::mat4 waterModelMatrix;

	GLuint waterShader = 0;

	GLuint waterVertexArrayObject = 0;
	GLuint waterVertexBufferObject = 0;
	GLuint waterIndexBufferObject = 0;

	GLuint reflectionTexture = 0;
	GLuint refractionTexture = 0;
	GLuint refractionDepthTexture = 0;

	GLuint dudvTexture = 0;
	GLuint normalMap = 0;

	float tiling;

	float waveSpeed;
	float moveFactor = 0.0f;
	float waveStrength;

	float shineDamper;
	float reflectivity;

	float distortionDampening;
	float highlightDampening;
	float borderTransparencyFactor;

	float fresnelModifier;

	float normalFlattenFactor;

	float murkyColourFactor;
	glm::vec4 murkyColour;
	float blueTintFactor;
	glm::vec4 blueColour;

	glm::vec3 sunDirection;

	void setLevel(float newHeight);
};

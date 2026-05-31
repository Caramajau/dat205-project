#include "water.h"

void Water::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/water.vert", "../project/water.frag", is_reload);
	if (shader != 0) {
		waterShader = shader;
	}

	loadTexture(dudvTexture, "../scenes/textures/waterDuDv.png");
	loadTexture(normalMap, "../scenes/textures/waterNormal.png");
}

void Water::setGpuData(const ProceduralConfig& config, const WaterFrameBuffers& waterFBOs) {
	// Account for the terrain being [0, width) and [0, length)
	int terrainWidth = config.width - 1;
	int terrainLength = config.length - 1;

	reflectionTexture = waterFBOs.getReflectionTexture();
	refractionTexture = waterFBOs.getRefractionTexture();
	refractionDepthTexture = waterFBOs.getRefractionDepthTexture();

	float vertices[] = {
		0.0f,			0.0f, 0.0f,			 0.0f,			0.0f,
		terrainWidth,	0.0f, 0.0f,			 terrainWidth,	0.0f,
		terrainWidth,	0.0f, terrainLength, terrainWidth,	terrainLength,
		0.0f,			0.0f, terrainLength, 0.0f,			terrainLength
	};

	unsigned int indices[] = {
		0, 2, 1,
		0, 3, 2
	};

	glGenVertexArrays(1, &waterVertexArrayObject);
	glBindVertexArray(waterVertexArrayObject);

	glGenBuffers(1, &waterVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, waterVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &waterIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), nullptr);
	// Texture coord attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

void Water::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, float deltaTime, const glm::vec3& cameraPosition, float near, float far, const glm::vec3& lightPosition, const ProceduralConfig& config) {
	moveFactor += config.waterWaveSpeed * deltaTime;
	// Loop back
	if (moveFactor > 1.0f)
	{
		moveFactor -= 1.0f;
	}

	glUseProgram(waterShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflectionTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refractionTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dudvTexture);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normalMap);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);

	waterModelMatrix = translate(config.waterLevel * glm::vec3(0.0f, 1.0f, 0.0f));
	labhelper::setUniformSlow(waterShader, "modelMatrix", waterModelMatrix);
	labhelper::setUniformSlow(waterShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * waterModelMatrix);

	labhelper::setUniformSlow(waterShader, "cameraPosition", cameraPosition);
	labhelper::setUniformSlow(waterShader, "usePointLight", config.usePointLight);
	labhelper::setUniformSlow(waterShader, "sunDirection", config.sunDirection);
	labhelper::setUniformSlow(waterShader, "lightPosition", lightPosition);

	labhelper::setUniformSlow(waterShader, "moveFactor", moveFactor);
	labhelper::setUniformSlow(waterShader, "waveStrength", config.waterWaveStrength);

	labhelper::setUniformSlow(waterShader, "shineDamper", config.waterShineDamper);
	labhelper::setUniformSlow(waterShader, "reflectivity", config.waterReflectivity);

	labhelper::setUniformSlow(waterShader, "borderTransparencyFactor", config.waterBorderTransparencyFactor);
	labhelper::setUniformSlow(waterShader, "distortionDampening", config.waterDistortionDampening);
	labhelper::setUniformSlow(waterShader, "highlightDampening", config.waterHighlightDampening);

	labhelper::setUniformSlow(waterShader, "near", near);
	labhelper::setUniformSlow(waterShader, "far", far);

	labhelper::setUniformSlow(waterShader, "tiling", config.waterTiling);

	labhelper::setUniformSlow(waterShader, "fresnelModifier", config.waterFresnelModifier);

	labhelper::setUniformSlow(waterShader, "normalFlattenFactor", config.waterNormalFlattenFactor);

	labhelper::setUniformSlow(waterShader, "murkyColourFactor", config.waterMurkyColourFactor);
	
	glm::vec4 murkyColour = config.waterMurkyColour;
	glUniform4f(glGetUniformLocation(waterShader, "murkyColour"),
		murkyColour.r, murkyColour.g, murkyColour.b, murkyColour.a);

	glm::vec4 blueColour = config.waterTintColour;
	labhelper::setUniformSlow(waterShader, "tintFactor", config.waterTintFactor);
	glUniform4f(glGetUniformLocation(waterShader, "tintColour"),
		blueColour.r, blueColour.g, blueColour.b, blueColour.a);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(waterVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

#include "water.h"

void Water::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/water.vert", "../project/water.frag", is_reload);
	if (shader != 0) {
		waterShader = shader;
	}

	loadDuDvTexture(dudvTexture, "../scenes/textures/waterDuDv.png");
	loadDuDvTexture(normalMap, "../scenes/textures/waterNormal.png");
}

void Water::loadDuDvTexture(GLuint& texture, const char* filepath) const {
	int w;
	int h;
	int comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	stbi_image_free(image);
}

void Water::setGpuData(const ProceduralConfig& config, const WaterFrameBuffers& waterFBOs) {
	int terrainWidth = config.width;
	int terrainHeight = config.height;
	reflectionTexture = waterFBOs.getReflectionTexture();
	refractionTexture = waterFBOs.getRefractionTexture();
	refractionDepthTexture = waterFBOs.getRefractionDepthTexture();
	sunDirection = config.sunDirection;
	setHeight(config.waterHeight);

	float vertices[] = {
		0.0f,			0.0f, 0.0f,			 0.0f, 0.0f,
		terrainWidth,	0.0f, 0.0f,			 1.0f, 0.0f,
		terrainWidth,	0.0f, terrainHeight, 1.0f, 1.0f,
		0.0f,			0.0f, terrainHeight, 0.0f, 1.0f
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

void Water::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, float deltaTime, const glm::vec3& cameraPosition) {
	moveFactor += waveSpeed * deltaTime;
	// Loop back
	if (moveFactor > 1.0f)
	{
		moveFactor -= 1.0f;
	}

	glUseProgram(waterShader);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, reflectionTexture);
	glUniform1i(glGetUniformLocation(waterShader, "reflectionTexture"), 14);

	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_2D, refractionTexture);
	glUniform1i(glGetUniformLocation(waterShader, "refractionTexture"), 15);

	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, dudvTexture);
	glUniform1i(glGetUniformLocation(waterShader, "dudvMap"), 16);

	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glUniform1i(glGetUniformLocation(waterShader, "normalMap"), 17);

	glActiveTexture(GL_TEXTURE18);
	glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);
	glUniform1i(glGetUniformLocation(waterShader, "depthMap"), 18);

	labhelper::setUniformSlow(waterShader, "modelMatrix", waterModelMatrix);
	labhelper::setUniformSlow(waterShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * waterModelMatrix);
	labhelper::setUniformSlow(waterShader, "moveFactor", moveFactor);
	labhelper::setUniformSlow(waterShader, "cameraPosition", cameraPosition);
	labhelper::setUniformSlow(waterShader, "sunDirection", sunDirection);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(waterVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

float Water::getHeight() const
{
	return height;
}

void Water::setHeight(float newHeight)
{
	height = newHeight;
	waterModelMatrix = translate(height * glm::vec3(0.0f, 1.0f, 0.0f));
}

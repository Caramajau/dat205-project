#include "water.h"

void Water::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/water.vert", "../project/water.frag", is_reload);
	if (shader != 0) {
		waterShader = shader;
	}
}

void Water::setGpuData(const ProceduralConfig& config) {
	int terrainWidth = config.width;
	int terrainHeight = config.height;

	float vertices[] = {
		0.0f,			0.0f, 0.0f,			0.0f, 0.0f,
		terrainWidth,	0.0f, 0.0f,			1.0f, 0.0f,
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

void Water::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const {
	glUseProgram(waterShader);

	labhelper::setUniformSlow(waterShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * waterModelMatrix);

	glBindVertexArray(waterVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

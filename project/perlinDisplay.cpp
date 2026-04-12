#include "perlinDisplay.h"

PerlinDisplay::PerlinDisplay() = default;

PerlinDisplay::~PerlinDisplay() = default;

void PerlinDisplay::loadShader(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../project/perlin.vert", "../project/perlin.frag", is_reload);
	if (shader != 0) {
		perlinShader = shader;
	}
}

void PerlinDisplay::initGpuData(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType)
{
	grid = createPerlinGrid(perlinWidth, perlinHeight, gridSize, octaveCount, lacunarity, persistence, interpolationType);

	// Positions (x, y, z) and texture coords (u, v)
	float quadVertices[] = {
		//  x,     y,   z,    u,   v
		-50.0f, -50.0f, 0.0f,  0.0f, 0.0f,
		 50.0f, -50.0f, 0.0f,  1.0f, 0.0f,
		 50.0f,  50.0f, 0.0f,  1.0f, 1.0f,
		-50.0f,  50.0f, 0.0f,  0.0f, 1.0f
	};

	unsigned int quadIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &quadVertexArrayObject);
	glBindVertexArray(quadVertexArrayObject);

	glGenBuffers(1, &quadVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &quadIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), nullptr);
	// Texture coord attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, perlinWidth, perlinHeight, 0, GL_RED, GL_FLOAT, grid.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void PerlinDisplay::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const
{
	glUseProgram(perlinShader);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glUniform1i(glGetUniformLocation(perlinShader, "perlinTex"), 7);

	labhelper::setUniformSlow(perlinShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * perlinNoiseModelMatrix);

	glBindVertexArray(quadVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

// TODO: Similar case to proceduralTerrain
void PerlinDisplay::reloadTexture(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType)
{
	grid = createPerlinGrid(perlinWidth, perlinHeight, gridSize, octaveCount, lacunarity, persistence, interpolationType);

	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, perlinWidth, perlinHeight, GL_RED, GL_FLOAT, grid.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

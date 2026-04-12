#include "proceduralTerrain.h"

ProceduralTerrain::ProceduralTerrain() = default;
ProceduralTerrain::~ProceduralTerrain() = default;

void ProceduralTerrain::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/procedural.vert", "../project/procedural.frag", is_reload);
	if (shader != 0) {
		terrainShader = shader;
	}
}

void ProceduralTerrain::initGpuData(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType, float initialHeightScale) {
	heightMapGrid = createPerlinGrid(perlinWidth, perlinHeight, gridSize, octaveCount, lacunarity, persistence, interpolationType);
	heightScale = initialHeightScale;

	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, perlinWidth, perlinHeight, 0, GL_RED, GL_FLOAT, heightMapGrid.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<float> vertices = createVertices(perlinWidth, perlinHeight);

	std::vector<unsigned int> indices = createIndices(perlinWidth, perlinHeight);

	triangleCount = indices.size();

	glGenVertexArrays(1, &terrainVertexArrayObject);
	glBindVertexArray(terrainVertexArrayObject);

	glGenBuffers(1, &terrainVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &terrainIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

std::vector<float> ProceduralTerrain::createVertices(int width, int height) const {
	std::vector<float> vertices;

	for (int z = 0; z < height; z++) {
		for (int x = 0; x < width; x++) {
			float fx = (float)x / width;
			float fz = (float)z / height;

			// The terrain starts flat at y = 0
			vertices.push_back(x);
			vertices.push_back(0);
			vertices.push_back(z);
			vertices.push_back(fx);
			vertices.push_back(fz);
		}
	}

	return vertices;
}

std::vector<unsigned int> ProceduralTerrain::createIndices(int width, int height) const {
	std::vector<unsigned int> indices;

	// Then there should be two triangles per quad.
	for (int z = 0; z < height - 1; z++) {
		for (int x = 0; x < width - 1; x++) {
			unsigned int topLeft = x + z * width;
			unsigned int topRight = (x + 1) + z * width;
			unsigned int bottomLeft = x + (z + 1) * width;
			unsigned int bottomRight = (x + 1) + (z + 1) * width;

			// First triangle
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			// Second triangle
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

	return indices;
}

void ProceduralTerrain::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const {
	glUseProgram(terrainShader);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glUniform1i(glGetUniformLocation(terrainShader, "heightMap"), 8);

	labhelper::setUniformSlow(terrainShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * terrainModelMatrix);
	labhelper::setUniformSlow(terrainShader, "heightScale", heightScale);

	glBindVertexArray(terrainVertexArrayObject);
	glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

// TODO: To allow changes to the size, could probably just use the init gpu data method and remove this one
void ProceduralTerrain::reloadTexture(int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType, float newHeightScale) {
	heightMapGrid = createPerlinGrid(perlinWidth, perlinHeight, gridSize, octaveCount, lacunarity, persistence, interpolationType);

	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, perlinWidth, perlinHeight, GL_RED, GL_FLOAT, heightMapGrid.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	heightScale = newHeightScale;
}

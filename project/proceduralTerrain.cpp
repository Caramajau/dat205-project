#include "proceduralTerrain.h"
#include <stb_image.h>

ProceduralTerrain::ProceduralTerrain() = default;
ProceduralTerrain::~ProceduralTerrain() = default;

void ProceduralTerrain::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/procedural.vert", "../project/procedural.frag", is_reload);
	if (shader != 0) {
		terrainShader = shader;
	}

	loadTerrainTexture(grassTexture, "../scenes/textures/grass.jpg");
	loadTerrainTexture(rockTexture, "../scenes/textures/rock.jpg");
	loadTerrainTexture(grassNormalMap, "../scenes/textures/grassNormal.jpg");
	loadTerrainTexture(rockNormalMap, "../scenes/textures/rockNormal.jpg");
}

void ProceduralTerrain::loadTerrainTexture(GLuint& texture, const char* filepath) const {
	int w;
	int h;
	int comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
	//init texture data
	glGenTextures(1, &texture);
	//bind texture, allocate storage and upload the data
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

void ProceduralTerrain::setGpuData(const ProceduralConfig& config) {
	heightMapGrid = createHeightMap(config);
	heightScale = config.heightScale;
	gridSize = config.gridSize;
	useNeighbours = config.useNeighbours;
	sunDirection = config.sunDirection;

	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, config.width, config.height, 0, GL_RED, GL_FLOAT, heightMapGrid.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<float> vertices = createVertices(config.width, config.height);

	std::vector<unsigned int> indices = createIndices(config.width, config.height);

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

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glUniform1i(glGetUniformLocation(terrainShader, "grassTexture"), 9);

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, rockTexture);
	glUniform1i(glGetUniformLocation(terrainShader, "rockTexture"), 10);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, grassNormalMap);
	glUniform1i(glGetUniformLocation(terrainShader, "grassNormalMap"), 11);

	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, rockNormalMap);
	glUniform1i(glGetUniformLocation(terrainShader, "rockNormalMap"), 12);

	labhelper::setUniformSlow(terrainShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * terrainModelMatrix);
	labhelper::setUniformSlow(terrainShader, "heightScale", heightScale);
	labhelper::setUniformSlow(terrainShader, "gridSize", gridSize);
	labhelper::setUniformSlow(terrainShader, "useNeighbours", useNeighbours);
	labhelper::setUniformSlow(terrainShader, "sunDirection", sunDirection);

	glBindVertexArray(terrainVertexArrayObject);
	glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

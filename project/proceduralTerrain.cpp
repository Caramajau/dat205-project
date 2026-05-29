#include "proceduralTerrain.h"

ProceduralTerrain::ProceduralTerrain() = default;
ProceduralTerrain::~ProceduralTerrain() = default;

void ProceduralTerrain::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/procedural.vert", "../project/procedural.frag", is_reload);
	if (shader != 0) {
		terrainShader = shader;
	}

	// Texture from https://ambientcg.com/a/Grass005
	loadTexture(grassTexture, "../scenes/textures/grass.jpg");
	loadTexture(grassNormalMap, "../scenes/textures/grassNormal.jpg");

	// Texture from https://ambientcg.com/a/Ground067
	loadTexture(rockTexture, "../scenes/textures/rock.jpg");
	loadTexture(rockNormalMap, "../scenes/textures/rockNormal.jpg");

	// Texture from https://ambientcg.com/a/Ground033
	loadTexture(sandTexture, "../scenes/textures/sand.jpg");
	loadTexture(sandNormalMap, "../scenes/textures/sandNormal.jpg");

	// Texture from https://ambientcg.com/a/Snow010A
	loadTexture(snowTexture, "../scenes/textures/snow.jpg");
	loadTexture(snowNormalMap, "../scenes/textures/snowNormal.jpg");
}

void ProceduralTerrain::setGpuData(const ProceduralConfig& config) {
	heightMapGrid = createHeightMap(config);
	setLevel(config.terrainLevel);

	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, config.width, config.length, 0, GL_RED, GL_FLOAT, heightMapGrid.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::vector<float> vertices = createVertices(config.width, config.length);

	std::vector<unsigned int> indices = createIndices(config.width, config.length);

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

std::vector<float> ProceduralTerrain::createVertices(int width, int length) const {
	std::vector<float> vertices;

	for (int z = 0; z < length; z++) {
		for (int x = 0; x < width; x++) {
			float fx = (float)x / width;
			float fz = (float)z / length;

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

std::vector<unsigned int> ProceduralTerrain::createIndices(int width, int length) const {
	std::vector<unsigned int> indices;

	// Then there should be two triangles per quad.
	for (int z = 0; z < length - 1; z++) {
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

void ProceduralTerrain::submitToGpu(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec4& waterPlane, float waterLevel, glm::vec3 lightPosition, const ProceduralConfig& config) const {
	glUseProgram(terrainShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, grassNormalMap);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, rockTexture);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, rockNormalMap);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, sandTexture);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, sandNormalMap);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, snowTexture);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, snowNormalMap);

	labhelper::setUniformSlow(terrainShader, "modelMatrix", terrainModelMatrix);
	labhelper::setUniformSlow(terrainShader, "modelViewProjectionMatrix", projMatrix * viewMatrix * terrainModelMatrix);

	labhelper::setUniformSlow(terrainShader, "heightScale", config.heightScale);

	labhelper::setUniformSlow(terrainShader, "useNeighbours", config.useNeighbours);
	labhelper::setUniformSlow(terrainShader, "usePointLight", config.usePointLight);
	labhelper::setUniformSlow(terrainShader, "sunDirection", config.sunDirection);
	labhelper::setUniformSlow(terrainShader, "lightPosition", lightPosition);

	glUniform4f(glGetUniformLocation(terrainShader, "waterPlane"),
		waterPlane.x, waterPlane.y, waterPlane.z, waterPlane.w);

	labhelper::setUniformSlow(terrainShader, "textureZoom", config.textureZoom);

	labhelper::setUniformSlow(terrainShader, "grassThreshold", config.grassThreshold);
	labhelper::setUniformSlow(terrainShader, "rockThreshold", config.rockThreshold);
	labhelper::setUniformSlow(terrainShader, "sandThreshold", config.sandThreshold);
	labhelper::setUniformSlow(terrainShader, "sandLevelOffset", config.sandLevelOffset);
	labhelper::setUniformSlow(terrainShader, "snowThreshold", config.snowThreshold);
	labhelper::setUniformSlow(terrainShader, "snowStartLevelOffset", config.snowStartLevelOffset);
	labhelper::setUniformSlow(terrainShader, "snowEndLevelOffset", config.snowEndLevelOffset);

	labhelper::setUniformSlow(terrainShader, "waterLevel", waterLevel - terrainLevel);

	labhelper::setUniformSlow(terrainShader, "triplanarBlendFactor", config.triplanarBlendFactor);

	glBindVertexArray(terrainVertexArrayObject);
	glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

float ProceduralTerrain::getLevel() const {
	return terrainLevel;
}

void ProceduralTerrain::setLevel(float newLevel) {
	terrainLevel = newLevel;
	terrainModelMatrix = translate(terrainLevel * glm::vec3(0.0f, 1.0f, 0.0f));
}


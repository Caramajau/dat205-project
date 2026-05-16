#include "waterFrameBuffers.h"

WaterFrameBuffers::WaterFrameBuffers() = default;

// call when closing the "game"
WaterFrameBuffers::~WaterFrameBuffers() {
	glDeleteFramebuffers(1, &reflectionFrameBuffer);
	glDeleteTextures(1, &reflectionTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);

	glDeleteFramebuffers(1, &refractionFrameBuffer);
	glDeleteTextures(1, &refractionTexture);
	glDeleteTextures(1, &refractionDepthTexture);
}

// call when loading the "game"
void WaterFrameBuffers::initialise() {
	initialiseReflectionFrameBuffer();
	initialiseRefractionFrameBuffer();
}

void WaterFrameBuffers::initialiseReflectionFrameBuffer() {
	reflectionFrameBuffer = createFrameBuffer();
	reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	unbindCurrentFrameBuffer(0, 0);
}

void WaterFrameBuffers::initialiseRefractionFrameBuffer() {
	refractionFrameBuffer = createFrameBuffer();
	refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	unbindCurrentFrameBuffer(0, 0);
}

// call to switch to default frame buffer
void WaterFrameBuffers::unbindCurrentFrameBuffer(int windowWidth, int windowHeight) const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

GLuint WaterFrameBuffers::createFrameBuffer() const {
	GLuint frameBuffer;
	// generate name for frame buffer
	glGenFramebuffers(1, &frameBuffer);

	// create the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// indicate that we will always render to colour attachment 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	return frameBuffer;
}

GLuint WaterFrameBuffers::createTextureAttachment(int width, int height) const {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Adds texture to currently bound framebuffer object
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
	return texture;
}

GLuint WaterFrameBuffers::createDepthTextureAttachment(int width, int height) const {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Adds texture to currently bound framebuffer object
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	return texture;
}

// Depth buffer attachment that isn't a texture, but rather a render buffer
GLuint WaterFrameBuffers::createDepthBufferAttachment(int width, int height) const {
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}

// call before rendering to this FBO
void WaterFrameBuffers::bindReflectionFrameBuffer() const {
	bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

// call before rendering to this FBO
void WaterFrameBuffers::bindRefractionFrameBuffer() const {
	bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

// To be able to tell OpenGL to render to own frame buffer objects
void WaterFrameBuffers::bindFrameBuffer(GLuint frameBuffer, int width, int height) const {
	// To make sure the texture isn't bound
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// Change resolution of viewport to match selected framebuffer object.
	glViewport(0, 0, width, height);
}

// get the resulting texture
GLuint WaterFrameBuffers::getReflectionTexture() const {
	return reflectionTexture;
}

// get the resulting texture
GLuint WaterFrameBuffers::getRefractionTexture() const {
	return refractionTexture;
}

// get the resulting depth texture
GLuint WaterFrameBuffers::getRefractionDepthTexture() const {
	return refractionDepthTexture;
}

// The methods below are only for seeing the content in the framebuffer objects, 
// i.e. only intended for debugging
void WaterFrameBuffers::loadShader(bool is_reload) {
	GLuint shader = labhelper::loadShaderProgram("../project/debugWaterFrameBuffers.vert", "../project/debugWaterFrameBuffers.frag", is_reload);
	if (shader != 0) {
		waterDebugShader = shader;
	}
}

// Setup to draw frame buffer content in corner
void WaterFrameBuffers::setGpuData(GLuint texture) {
	debugTexture = texture;

	float vertices[] = {
		-1.0f, -1.0f,   0.0f, 0.0f,
		-0.5f, -1.0f,   1.0f, 0.0f,
		-0.5f, -0.5f,   1.0f, 1.0f,
		-1.0f, -0.5f,   0.0f, 1.0f,
	};

	unsigned int indices[] = { 
		0, 1, 2, 
		2, 3, 0 
	};

	glGenVertexArrays(1, &debugVertexArrayObject);
	glGenBuffers(1, &debugVertexBufferObject);
	glGenBuffers(1, &debugIndexBufferObject);

	glBindVertexArray(debugVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, debugVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void WaterFrameBuffers::submitToGpu() const {
	glUseProgram(waterDebugShader);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, debugTexture);
	glUniform1i(glGetUniformLocation(waterDebugShader, "debugTexture"), 13);

	glBindVertexArray(debugVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

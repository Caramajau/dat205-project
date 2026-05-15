#include "waterFrameBuffers.h"

// call when loading the game
WaterFrameBuffers::WaterFrameBuffers() {
	initialiseReflectionFrameBuffer();
	initialiseRefractionFrameBuffer();
}

// call when closing the game
WaterFrameBuffers::~WaterFrameBuffers() {
	glDeleteFramebuffers(1, &reflectionFrameBuffer);
	glDeleteTextures(1, &reflectionTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);

	glDeleteFramebuffers(1, &refractionFrameBuffer);
	glDeleteTextures(1, &refractionTexture);
	glDeleteTextures(1, &refractionDepthTexture);
}

void WaterFrameBuffers::initialiseReflectionFrameBuffer() {
	reflectionFrameBuffer = createFrameBuffer();
	reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	unbindCurrentFrameBuffer();
}

void WaterFrameBuffers::initialiseRefractionFrameBuffer() {
	refractionFrameBuffer = createFrameBuffer();
	refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	unbindCurrentFrameBuffer();
}

// call to switch to default frame buffer
void WaterFrameBuffers::unbindCurrentFrameBuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glViewport(0, 0, windowWidth, windowHeight);
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

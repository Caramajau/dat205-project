#pragma once

#include <GL/glew.h>
#include <labhelper.h>

// In general, many parts of the water implementation are based on: https://www.youtube.com/watch?v=HusvGeEDU_U&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh
// Specifically, this class is based on: https://www.dropbox.com/scl/fo/x651re40kl35x0wacurgm/AAlMZSHg8DSlzBqnHUVeNQk?rlkey=jmdh3k6ibjgi9o8eq5nev628r&e=1&dl=0

class WaterFrameBuffers {
public:
	explicit WaterFrameBuffers();
	~WaterFrameBuffers();

	void initialise();

	void bindReflectionFrameBuffer() const;
	void bindRefractionFrameBuffer() const;

	void unbindCurrentFrameBuffer(int windowWidth, int windowHeight) const;

	GLuint getReflectionTexture() const;
	GLuint getRefractionTexture() const;
	GLuint getRefractionDepthTexture() const;

	void loadShader(bool is_reload);
	void setGpuData(GLuint texture);
	void submitToGpu() const;

private:
	// Resolutions could be changed, higher is more expensive.
	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;

	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;

	GLuint reflectionFrameBuffer = 0;
	GLuint reflectionTexture = 0;
	GLuint reflectionDepthBuffer = 0;

	GLuint refractionFrameBuffer = 0;
	GLuint refractionTexture = 0;
	GLuint refractionDepthTexture = 0;

	GLuint debugTexture = 0;
	GLuint waterDebugShader = 0;

	GLuint debugVertexArrayObject = 0;
	GLuint debugVertexBufferObject = 0;
	GLuint debugIndexBufferObject = 0;

	void initialiseReflectionFrameBuffer();
	void initialiseRefractionFrameBuffer();

	void bindFrameBuffer(GLuint frameBuffer, int width, int height) const;

	GLuint createFrameBuffer() const;

	GLuint createTextureAttachment(int width, int height) const;
	GLuint createDepthTextureAttachment(int width, int height) const;
	GLuint createDepthBufferAttachment(int width, int height) const;
};

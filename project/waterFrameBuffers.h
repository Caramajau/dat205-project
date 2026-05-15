#pragma once

#include <GL/glew.h>

// Class based on: https://www.dropbox.com/scl/fo/x651re40kl35x0wacurgm/AAlMZSHg8DSlzBqnHUVeNQk?rlkey=jmdh3k6ibjgi9o8eq5nev628r&e=1&dl=0

class WaterFrameBuffers {
public:
	explicit WaterFrameBuffers();
	~WaterFrameBuffers();

	void bindReflectionFrameBuffer() const;
	void bindRefractionFrameBuffer() const;

	void unbindCurrentFrameBuffer() const;

	GLuint getReflectionTexture() const;
	GLuint getRefractionTexture() const;
	GLuint getRefractionDepthTexture() const;

private:
	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;

	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;

	GLuint reflectionFrameBuffer;
	GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;

	GLuint refractionFrameBuffer;
	GLuint refractionTexture;
	GLuint refractionDepthTexture;

	void initialiseReflectionFrameBuffer();

	void initialiseRefractionFrameBuffer();

	void bindFrameBuffer(GLuint frameBuffer, int width, int height) const;

	GLuint createFrameBuffer() const;

	GLuint createTextureAttachment(int width, int height) const;

	GLuint createDepthTextureAttachment(int width, int height) const;

	GLuint createDepthBufferAttachment(int width, int height) const;
};

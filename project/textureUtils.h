#pragma once

#include <GL/glew.h>
#include <stb_image.h>
#include <iostream>

void loadTexture(GLuint& texture, const char* filepath, float anisotropyLevel = 16.0f);

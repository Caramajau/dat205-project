#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordsIn;
uniform mat4 modelViewProjectionMatrix;

out vec4 clipSpace;
out vec2 texCoords;

const float tiling = 6.0;

void main()
{
	clipSpace = modelViewProjectionMatrix * vec4(position, 1.0);
	gl_Position = clipSpace;
	texCoords = texCoordsIn * tiling;
}

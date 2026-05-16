#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordsIn;
uniform mat4 modelViewProjectionMatrix;

out vec2 texCoords;

void main()
{
	texCoords = texCoordsIn;
	gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
}

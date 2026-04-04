#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

uniform mat4 modelViewProjectionMatrix;

// Pass to fragment shader
out vec2 texCoord;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
	texCoord = texCoordIn;
}

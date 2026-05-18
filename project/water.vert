#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordsIn;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec3 cameraPosition;

out vec4 clipSpace;
out vec2 texCoords;
out vec3 toCameraVector;

const float tiling = 4.0;

void main()
{
	vec4 worldPosition = modelMatrix * vec4(position, 1.0);
	clipSpace = modelViewProjectionMatrix * vec4(position, 1.0);
	gl_Position = clipSpace;
	texCoords = texCoordsIn * tiling;
	toCameraVector = cameraPosition - worldPosition.xyz;
}

#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordsIn;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec3 cameraPosition;
uniform float tiling;
uniform vec3 lightPosition;

out vec4 clipSpace;
out vec2 texCoords;
out vec3 toCameraVector;
out vec3 fromLightVector;

void main()
{
	vec4 worldPosition = modelMatrix * vec4(position, 1.0);
	clipSpace = modelViewProjectionMatrix * vec4(position, 1.0);
	gl_Position = clipSpace;
	texCoords = texCoordsIn * tiling;
	toCameraVector = cameraPosition - worldPosition.xyz;
	fromLightVector = worldPosition.xyz - lightPosition;
}

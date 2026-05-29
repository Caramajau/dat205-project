#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform sampler2D heightMap;
uniform float heightScale;

uniform vec3 lightPosition;

out vec3 positionWithHeight;
out vec2 texCoord;
out vec3 fromLightVector;

uniform vec4 waterPlane;

void main()
{
	float height = texture(heightMap, texCoordIn).r;
	texCoord = texCoordIn;
	positionWithHeight = vec3(position.x, height * heightScale, position.z);
	vec4 worldPosition = modelMatrix * vec4(positionWithHeight, 1.0);
	fromLightVector = worldPosition.xyz - lightPosition;

	gl_ClipDistance[0] = dot(worldPosition, waterPlane);
	gl_Position = modelViewProjectionMatrix * vec4(positionWithHeight, 1.0);
}

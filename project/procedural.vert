#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

uniform mat4 modelViewProjectionMatrix;
uniform sampler2D heightMap;
uniform float heightScale;

out float height;

void main()
{
	height = texture(heightMap, texCoordIn).r;
	vec3 positionWithHeight = vec3(position.x, height * heightScale, position.z);
	gl_Position = modelViewProjectionMatrix * vec4(positionWithHeight, 1.0);
}

#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

uniform mat4 modelViewProjectionMatrix;
uniform sampler2D heightMap;

void main()
{
	float height = texture(heightMap, texCoordIn).r;
	vec3 positionWithHeight = vec3(position.x, height, position.z);
	gl_Position = modelViewProjectionMatrix * vec4(positionWithHeight, 1.0);
}

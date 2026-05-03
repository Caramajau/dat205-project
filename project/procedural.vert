#version 420

// Input vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

// Input uniform variables
uniform mat4 modelViewProjectionMatrix;
uniform sampler2D heightMap;
uniform float heightScale;

uniform mat4 modelViewMatrix;

// Output to fragment shader
out vec3 positionWithHeight;
out vec2 texCoord;
out vec3 viewSpacePosition;

// Moved shadow stuff to vertex shader from fragment
uniform mat4 lightMatrix;
out vec4 shadowMapCoord;

void main()
{
	// Terrain stuff
	float height = texture(heightMap, texCoordIn).r;
	positionWithHeight = vec3(position.x, height * heightScale, position.z);
	gl_Position = modelViewProjectionMatrix * vec4(positionWithHeight, 1.0);

	// Related to shadows and PBS
	texCoord = texCoordIn;
	viewSpacePosition = (modelViewMatrix * vec4(position, 1.0)).xyz;
	shadowMapCoord = lightMatrix * vec4(viewSpacePosition, 1.f);
}

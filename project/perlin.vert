#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoordIn;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;

// Pass to fragment shader
out vec2 texCoord;

uniform vec4 waterPlane;

void main()
{
	gl_ClipDistance[0] = dot(modelMatrix * vec4(position, 1.0), waterPlane);
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
	texCoord = texCoordIn;
}

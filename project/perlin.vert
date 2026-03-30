#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

uniform mat4 modelViewProjectionMatrix;

out vec2 v_texcoord; // Pass to fragment shader

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
    v_texcoord = texcoord;
}

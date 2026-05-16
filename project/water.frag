#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

in vec2 texCoords;

void main()
{
	vec4 reflectColour = texture(reflectionTexture, texCoords);
	vec4 refractColour = texture(refractionTexture, texCoords);
	
	fragmentColor = mix(reflectColour, refractColour, 0.5);
}

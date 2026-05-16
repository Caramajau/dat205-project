#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

in vec4 clipSpace;

void main()
{
	// Gives screen space points in [-1, 1]
	vec2 normalizedDeviceSpace = clipSpace.xy/clipSpace.w;
	// Change to [0, 1]
	normalizedDeviceSpace = normalizedDeviceSpace/2.0 + 0.5;

	// Y needs to be inverted for reflection
	vec2 reflectTexCoords = vec2(normalizedDeviceSpace.x, 1.0 - normalizedDeviceSpace.y);
	vec4 reflectColour = texture(reflectionTexture, reflectTexCoords);

	// Can be used directly.
	vec4 refractColour = texture(refractionTexture, normalizedDeviceSpace);
	
	fragmentColor = mix(reflectColour, refractColour, 0.5);
}

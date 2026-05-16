#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

in vec4 clipSpace;
in vec2 texCoords;

uniform float moveFactor;

// TODO: make customisable?
const float waveStrength = 0.02;

void main()
{
	// Gives screen space points in [-1, 1]
	vec2 normalizedDeviceSpace = clipSpace.xy / clipSpace.w;
	// Change to [0, 1]
	normalizedDeviceSpace = normalizedDeviceSpace / 2.0 + 0.5;

	vec2 refractTexCoords = vec2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);
	// Y needs to be inverted for reflection
	vec2 reflectTexCoords = vec2(normalizedDeviceSpace.x, 1.0 - normalizedDeviceSpace.y);

	// Distortion only in red and green and also stored in [0, 1], converted to [-1, 1]
	vec2 firstDistortion = (texture(dudvMap, vec2(texCoords.x + moveFactor, texCoords.y)).rg * 2.0 - 1.0) * waveStrength;
	vec2 secondDistortion = (texture(dudvMap, vec2(1.0 - texCoords.x, texCoords.y + moveFactor)).rg * 2.0 - 1.0) * waveStrength;
	vec2 totalDistortion = firstDistortion + secondDistortion;

	refractTexCoords += totalDistortion;
	// clamp to avoid wrap around glitch
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	reflectTexCoords += totalDistortion;
	reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);

	vec4 reflectColour = texture(reflectionTexture, reflectTexCoords);
	vec4 refractColour = texture(refractionTexture, refractTexCoords);
	
	fragmentColor = mix(reflectColour, refractColour, 0.5);
}

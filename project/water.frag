#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

uniform float moveFactor;

in vec4 clipSpace;
in vec2 texCoords;
in vec3 toCameraVector;

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

	// Distortion only in red and green
	vec2 distortedTexCoords = texture(dudvMap, vec2(texCoords.x + moveFactor, texCoords.y)).rg * 0.1;
	distortedTexCoords = texCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
	// Distortion is also stored as [0, 1], convert to [-1, 1]
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

	refractTexCoords += totalDistortion;
	// clamp to avoid wrap around glitch
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	reflectTexCoords += totalDistortion;
	reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);

	vec4 reflectColour = texture(reflectionTexture, reflectTexCoords);
	vec4 refractColour = texture(refractionTexture, refractTexCoords);
	
	vec3 viewVector = normalize(toCameraVector);
	// Assumes water normal is point up
	float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
	// Can increase/decrease the reflectiveness of the water, TODO: make customisable?
	refractiveFactor = pow(refractiveFactor, 2);

	fragmentColor = mix(reflectColour, refractColour, refractiveFactor);
	// Tint slightly blue
	fragmentColor = mix(fragmentColor, vec4(0.0, 0.3, 0.5, 1.0), 0.05);
}

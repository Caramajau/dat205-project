#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

layout(binding = 0) uniform sampler2D reflectionTexture;
layout(binding = 1) uniform sampler2D refractionTexture;
layout(binding = 2) uniform sampler2D dudvMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D depthMap;

uniform bool usePointLight;
uniform vec3 sunDirection;

uniform float moveFactor;
uniform float waveStrength;

uniform float shineDamper;
uniform float reflectivity;

uniform float distortionDampening;
uniform float highlightDampening;
uniform float borderTransparencyFactor;

uniform float fresnelModifier;

uniform float normalFlattenFactor;

uniform float murkyColourFactor;
uniform vec4 murkyColour;

uniform float blueTintFactor;
uniform vec4 blueColour;

uniform float near;
uniform float far;

in vec4 clipSpace;
in vec2 texCoords;
in vec3 toCameraVector;
in vec3 fromLightVector;

float calcTrueDepth(float depth)
{
	// Need to convert in order to get true depth
	// Explanation: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
	return 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
}

void main()
{
	// Gives screen space points in [-1, 1]
	vec2 normalizedDeviceSpace = clipSpace.xy / clipSpace.w;
	// Change to [0, 1]
	normalizedDeviceSpace = normalizedDeviceSpace / 2.0 + 0.5;

	vec2 refractTexCoords = vec2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);
	// Y needs to be inverted for reflection
	vec2 reflectTexCoords = vec2(normalizedDeviceSpace.x, 1.0 - normalizedDeviceSpace.y);

	// Important to sample before the coords get distorted
	float depth = texture(depthMap, refractTexCoords).r;

	float floorDistance = calcTrueDepth(depth);

	float waterDistance = calcTrueDepth(gl_FragCoord.z);

	float waterDepth = floorDistance - waterDistance;

	// First sample to get distortion value to use as texture coords for actual distortion (and normal later)
	// Distortion only in red and green
	vec2 distortedTexCoords = texture(dudvMap, vec2(texCoords.x + moveFactor, texCoords.y)).rg * 0.1;
	distortedTexCoords = texCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
	// Distortion is also stored as [0, 1], convert to [-1, 1]
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength * clamp(waterDepth / distortionDampening, 0.0, 1.0);

	refractTexCoords += totalDistortion;
	// clamp to avoid wrap around glitch
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	reflectTexCoords += totalDistortion;
	reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);

	vec4 reflectColour = texture(reflectionTexture, reflectTexCoords);
	vec4 refractColour = texture(refractionTexture, refractTexCoords);
	
	vec4 normalMapColour = texture(normalMap, distortedTexCoords);
	// Okay to not convert b, since you want normal pointing up to some extent anyways.
	// The normal flatten factor is used to make the normals point more up, making the water appear flatter.
	vec3 normal = vec3(normalMapColour.r * 2.0 - 1.0, normalMapColour.b * normalFlattenFactor, normalMapColour.g * 2.0 - 1.0);
	normal = normalize(normal);

	vec3 viewVector = normalize(toCameraVector);
	float fresnel = dot(viewVector, normal);
	// Can increase/decrease the reflectiveness of the water
	fresnel = pow(fresnel, fresnelModifier);
	// Clamp to avoid black artefacts
	fresnel = clamp(fresnel, 0.001, 0.999);

	vec3 lightSource = usePointLight ? fromLightVector : -sunDirection;
	vec3 reflectedLight = reflect(normalize(lightSource), normal);
	// Dot product to see if similar, and when they are:
	// that means more light into the camera thus brighter specular highlight.
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	// vec3(1.0) could be changed to different light colours
	vec3 specularHighlights = vec3(1.0) * specular * reflectivity * clamp(waterDepth / highlightDampening, 0.0, 1.0);

	// Murky colour effect
	refractColour = mix(refractColour, murkyColour, clamp(waterDepth / murkyColourFactor, 0.0, 1.0));

	fragmentColor = mix(reflectColour, refractColour, fresnel);
	// Tint slightly blue
	fragmentColor = mix(fragmentColor, blueColour, blueTintFactor) + vec4(specularHighlights, 0.0);

	fragmentColor.a = clamp(waterDepth / borderTransparencyFactor, 0.0, 1.0);
}

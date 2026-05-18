#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform vec3 sunDirection;

uniform float moveFactor;

in vec4 clipSpace;
in vec2 texCoords;
in vec3 toCameraVector;

// TODO: make customisable?
const float waveStrength = 0.04;
const float shineDamper = 20.0;
const float reflectivity = 0.5;

void main()
{
	// Gives screen space points in [-1, 1]
	vec2 normalizedDeviceSpace = clipSpace.xy / clipSpace.w;
	// Change to [0, 1]
	normalizedDeviceSpace = normalizedDeviceSpace / 2.0 + 0.5;

	vec2 refractTexCoords = vec2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);
	// Y needs to be inverted for reflection
	vec2 reflectTexCoords = vec2(normalizedDeviceSpace.x, 1.0 - normalizedDeviceSpace.y);

	// TODO: should be uniforms
	float near = 0.1;
	float far = 2000.0;

	// Important to sample before the coords get distorted
	float depth = texture(depthMap, refractTexCoords).r;

	// Need to convert in order to get true depth
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

	depth = gl_FragCoord.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

	float waterDepth = floorDistance - waterDistance;

	// First sample to get distortion value to use as texture coords for actual distortion (and normal later)
	// Distortion only in red and green
	vec2 distortedTexCoords = texture(dudvMap, vec2(texCoords.x + moveFactor, texCoords.y)).rg * 0.1;
	distortedTexCoords = texCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
	// Distortion is also stored as [0, 1], convert to [-1, 1], TODO: make distortion dampening value, 2.0, customisable?
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength * clamp(waterDepth / 2.0, 0.0, 1.0);

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

	vec4 normalMapColour = texture(normalMap, distortedTexCoords);
	// Okay to not convert b, since you want normal pointing up to some extent anyways.
	vec3 normal = vec3(normalMapColour.r * 2.0 - 1.0, normalMapColour.b, normalMapColour.g * 2.0 - 1.0);
	normal = normalize(normal);

	vec3 reflectedLight = reflect(normalize(-sunDirection), normal);
	// Dot product to see if similar, and when they are:
	// that means more light into the camera thus brighter specular highlight.
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	// vec3(1.0) could be changed to different light colours, TODO: make specular highlights dampening value, 2.0, customisable?
	vec3 specularHighlights = vec3(1.0) * specular * reflectivity * clamp(waterDepth / 2.0, 0.0, 1.0);

	fragmentColor = mix(reflectColour, refractColour, refractiveFactor);
	// Tint slightly blue
	fragmentColor = mix(fragmentColor, vec4(0.0, 0.3, 0.5, 1.0), 0.05) + vec4(specularHighlights, 0.0);

	// TODO: make depth value, 2.0, customisable?
	fragmentColor.a = clamp(waterDepth / 2.0, 0.0, 1.0);
}

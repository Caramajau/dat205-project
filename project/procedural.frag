#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

in vec3 positionWithHeight;

// Material
uniform vec3 material_color;
uniform float material_metalness;
uniform float material_fresnel;
uniform float material_shininess;
uniform vec3 material_emission;

//uniform int has_color_texture;
//layout(binding = 0) uniform sampler2D colorMap;
//uniform int has_emission_texture;
//layout(binding = 5) uniform sampler2D emissiveMap;

// Environment
layout(binding = 6) uniform sampler2D environmentMap;
layout(binding = 9) uniform sampler2D irradianceMap;
layout(binding = 10) uniform sampler2D reflectionMap;
uniform float environment_multiplier;

// Light source
uniform vec3 point_light_color = vec3(1.0, 1.0, 1.0);
uniform float point_light_intensity_multiplier = 50.0;

uniform vec3 viewSpaceLightDir;
uniform float spotOuterAngle;
uniform float spotInnerAngle;

uniform int useSpotLight;
uniform int useSoftFalloff;

// Shadow map stuff
in vec4 shadowMapCoord;
// layout(binding = 10) uniform sampler2D shadowMapTex;
layout(binding = 10) uniform sampler2DShadow shadowMapTex;

// Constants
#define PI 3.14159265359

// Input varyings from vertex shader
in vec2 texCoord;
in vec3 viewSpacePosition;

// Input uniform variables
uniform mat4 viewMatrix;
uniform mat4 viewInverse;
uniform vec3 viewSpaceLightPosition;

// Implementation used in the TDA362 labs.
vec3 calculateDirectIllumiunation(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 direct_illum = base_color;

	const float distance_to_light = length(viewSpaceLightPosition - viewSpacePosition); // d
	const float falloff_factor = 1.0/(distance_to_light*distance_to_light);
	vec3 Li = point_light_intensity_multiplier * point_light_color * falloff_factor;
	vec3 wi = normalize(viewSpaceLightPosition - viewSpacePosition);

	float dotProduct = dot(wi, n);
	if (dotProduct <= 0.0) {
		return vec3(0);
	}

    vec3 diffuse_term = base_color * 1.0/PI * length(dotProduct) * Li;

	vec3 wh = normalize(wo + wi);
	float fresnel_term = material_fresnel + ((1 - material_fresnel) * pow(1 - dot(wh, wi), 5));
	float microfacet_distribution_function = ((material_shininess + 2)/(2*PI))*pow(dot(n, wh), material_shininess);
	float shadowing_or_masking_function = min(1, min(2 * (dot(n, wh)*dot(n, wo))/dot(wo, wh), 2 * (dot(n, wh)*dot(n, wi))/dot(wo, wh)));

	float brdf = (fresnel_term*microfacet_distribution_function*shadowing_or_masking_function)/(4*dot(n, wo)*dot(n, wi));

	vec3 dieletric_term = (brdf * dot(n, wi) * Li) + ((1 - fresnel_term) * diffuse_term);
	vec3 metal_term = brdf * base_color * dot(n, wi) * Li;
	direct_illum = material_metalness*metal_term + (1 - material_metalness)*dieletric_term;

	return direct_illum;
}

// Implementation used in the TDA362 labs.
vec3 calculateIndirectIllumination(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 indirect_illum = vec3(0.f);

	vec4 world_space_normal = viewInverse * vec4(n, 0);
	float theta = acos(max(-1.0f, min(1.0f, world_space_normal.y)));
	float phi = atan(world_space_normal.z, world_space_normal.x);
	if(phi < 0.0f)
	{
		phi = phi + 2.0f * PI;
	}
	
	vec2 lookup = vec2(phi / (2.0 * PI), 1 - theta / PI);
	vec4 irradiance = environment_multiplier * texture(irradianceMap, lookup);

	vec3 diffuse_term = base_color * (1.0f / PI) * vec3(irradiance);

	vec3 wi = normalize(vec3(viewInverse*vec4(reflect(-wo, n), 0)));
	float roughness = sqrt(sqrt(2/(material_shininess+2)));
	vec3 Li = environment_multiplier * textureLod(reflectionMap, lookup, roughness*7).rgb; // 7???

	vec3 wh = normalize(wi + wo);
	float fresnel_term = material_fresnel + ((1 - material_fresnel) * pow(1 - dot(wh, wi), 5));

	vec3 dieletric_term = (fresnel_term * Li) + ((1 - fresnel_term) * diffuse_term);
	vec3 metal_term = fresnel_term * base_color * Li;
	indirect_illum = material_metalness*metal_term + (1-material_metalness)*dieletric_term;

	return indirect_illum;
}

void main()
{
    // Calculate normal to get correct lighting, idea from: https://www.youtube.com/shorts/gc7rT3sF1S8
	vec3 normal = normalize(cross(dFdx(positionWithHeight), dFdy(positionWithHeight)));
	vec3 viewSpaceNormal = normalize(viewMatrix * vec4(normal, 0.0)).xyz;

	// Perform transformation
	// float depth = texture(shadowMapTex, shadowMapCoord.xy / shadowMapCoord.w).x;
	// float visibility = (depth >= (shadowMapCoord.z / shadowMapCoord.w)) ? 1.0 : 0.0;
	float visibility = textureProj(shadowMapTex, shadowMapCoord);

	if (useSpotLight == 1)
	{
		vec3 posToLight = normalize(viewSpaceLightPosition - viewSpacePosition);
		float cosAngle = dot(posToLight, -viewSpaceLightDir);
		float spotAttenuation;
		// Spotlight with hard border:
		if (useSoftFalloff == 0)
		{
			spotAttenuation = (cosAngle > spotOuterAngle) ? 1.0 : 0.0;
		}
		else
		{
			// Spotlight that can interpolate between inner and outer angle.
			spotAttenuation = smoothstep(spotOuterAngle, spotInnerAngle, cosAngle);
		}
		visibility *= spotAttenuation;
	}


	vec3 wo = -normalize(viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);

	vec3 base_color = vec3(1.0);

	// Direct illumination
	vec3 direct_illumination_term = visibility * calculateDirectIllumiunation(wo, n, base_color);

	// Indirect illumination
	vec3 indirect_illumination_term = calculateIndirectIllumination(wo, n, base_color);

	///////////////////////////////////////////////////////////////////////////
	// Add emissive term. If emissive texture exists, sample this term.
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = vec3(0.0);

	vec3 shading = direct_illumination_term + indirect_illumination_term + emission_term;

	fragmentColor = vec4(shading, 1.0);
	return;
}

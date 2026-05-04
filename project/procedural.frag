#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform float heightScale;

in vec3 positionWithHeight;

void main()
{
    // Calculate normal to get correct lighting, idea from: https://www.youtube.com/shorts/gc7rT3sF1S8
	vec3 normal = normalize(cross(dFdx(positionWithHeight), dFdy(positionWithHeight)));

    float slope = normal.y;

    // Terrain colours (could be changed to use textures instead)
    // Maybe customise through GUI?
    vec3 grass = vec3(0.2, 0.6, 0.1);
    vec3 rock  = vec3(0.4, 0.3, 0.2);

    // NOTE: reversed order since the "slope" is based on the normal.
    float rockBlend = smoothstep(0.8, 0.6, slope);
    vec3 colour = mix(grass, rock, rockBlend);

    // Temporary sun direction, customise through GUI?
    vec3 sunDirection = normalize(vec3(0.8, 1.0, 0.6));
    float diffuse = max(dot(normal, sunDirection), 0.0);

    // Simple shading with some ambient and mostly diffuse
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(colour * light, 1.0);
}

#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

in vec3 positionWithHeight;

void main()
{
    // Calculate normal to get correct lighting, idea from: https://www.youtube.com/shorts/gc7rT3sF1S8
	vec3 normal = normalize(cross(dFdx(positionWithHeight), dFdy(positionWithHeight)));

    // Temporary sun direction
    vec3 sunDirection = normalize(vec3(0.8, 1.0, 0.6));
    float diffuse = max(dot(normal, sunDirection), 0.0);

    // Simple shading with some ambient and mostly diffuse
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(light, light, light, 1.0);
}

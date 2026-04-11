#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

in float height;

void main()
{
    // Uses the height value as colour for now.
    fragmentColor = vec4(height, height, height, 1.0);
}

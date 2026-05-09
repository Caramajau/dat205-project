#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform float heightScale;
uniform float gridSize;
uniform sampler2D heightMap;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
in vec2 texCoord;

in vec3 positionWithHeight;

uniform bool useNeighbours;
uniform vec3 sunDirection;

// Idea from https://www.youtube.com/shorts/gc7rT3sF1S8
vec3 positionNormal(vec3 position)
{
    vec3 normal = cross(dFdx(position), dFdy(position));
    return normalize(normal);
}

// Heavily inspired by https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/chapter5-andersson-terrain-rendering-in-frostbite.pdf
vec3 neighbourNormal(vec2 uv, float texelSize, float texelAspect)
{
    float hD = texture(heightMap, uv + texelSize*vec2( 0,-1)).r * texelAspect;
    float hL = texture(heightMap, uv + texelSize*vec2(-1, 0)).r * texelAspect;
    float hR = texture(heightMap, uv + texelSize*vec2( 1, 0)).r * texelAspect;
    float hU = texture(heightMap, uv + texelSize*vec2( 0, 1)).r * texelAspect;
    
    vec3 normal = vec3(hL - hR, 2.0, hD - hU);
    return normalize(normal);
}

void main()
{
    vec3 normal = useNeighbours ? neighbourNormal(texCoord, 1.0f/gridSize, heightScale) : positionNormal(positionWithHeight);
    
    float slope = 1 - normal.y;

    // 32 seemed like a good "zoom".
    // Texture from https://ambientcg.com/a/Grass005
    vec3 grass = texture(grassTexture, 32 * texCoord).rgb;
    // Texture from https://ambientcg.com/a/Ground067
    vec3 rock  = texture(rockTexture, 32 * texCoord).rgb;

    float rockBlend = smoothstep(0.2, 0.4, slope);
    vec3 colour = mix(grass, rock, rockBlend);

    float diffuse = max(dot(normal, normalize(sunDirection)), 0.0);

    // Simple shading with some ambient and mostly diffuse
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(colour * light, 1.0);
}

#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform float heightScale;
uniform sampler2D heightMap;

uniform float waterLevel;
uniform float sandLevelOffset;

uniform sampler2D grassTexture;
uniform sampler2D grassNormalMap;

uniform sampler2D rockTexture;
uniform sampler2D rockNormalMap;

uniform sampler2D sandTexture;
uniform sampler2D sandNormalMap;

uniform sampler2D snowTexture;
uniform sampler2D snowNormalMap;

uniform bool useNeighbours;
uniform vec3 sunDirection;

uniform float textureZoom;

uniform float grassThreshold;
uniform float rockThreshold;
uniform float sandThreshold;

uniform float triplanarBlendFactor;

in vec2 texCoord;
in vec3 positionWithHeight;

// Idea from https://www.youtube.com/shorts/gc7rT3sF1S8
vec3 positionNormal(vec3 position)
{
    vec3 normal = cross(dFdx(position), dFdy(position));
    return normalize(normal);
}

// Heavily inspired by https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/chapter5-andersson-terrain-rendering-in-frostbite.pdf
vec3 neighbourNormal(vec2 uv, float texelWidth, float texelHeight, float texelAspect)
{
    float hD = texture(heightMap, uv + texelHeight*vec2( 0,-1)).r * texelAspect;
    float hL = texture(heightMap, uv + texelWidth*vec2(-1, 0)).r * texelAspect;
    float hR = texture(heightMap, uv + texelWidth*vec2( 1, 0)).r * texelAspect;
    float hU = texture(heightMap, uv + texelHeight*vec2( 0, 1)).r * texelAspect;

    vec3 normal = vec3(hL - hR, 2.0, hD - hU);
    return normalize(normal);
}

// Unpack into a [-1,1] tangent space vector.
vec3 unpackNormal(sampler2D map, vec2 uv)
{
    return texture(map, uv).rgb * 2.0 - 1.0;
}

vec3 triplanarTexture(sampler2D tex, vec2 uvX, vec2 uvY, vec2 uvZ, vec3 weights)
{
    vec3 texX = texture(tex, uvX).rgb;
    vec3 texY = texture(tex, uvY).rgb;
    vec3 texZ = texture(tex, uvZ).rgb;

    return texX * weights.x + texY * weights.y + texZ * weights.z;
}

// Based on: https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a#da52
vec3 triplanarNormal(sampler2D map, vec2 uvX, vec2 uvY, vec2 uvZ, vec3 weights, vec3 terrainNormal)
{
    vec3 nX = unpackNormal(map, uvX);
    vec3 nY = unpackNormal(map, uvY);
    vec3 nZ = unpackNormal(map, uvZ);

    // Different blending approaches explained in: https://blog.selfshadow.com/publications/blending-in-detail/
    // - "Normally" whiteout blend is done in tangent space (z up): normalize(vec3(n1.xy + n2.xy, n1.z * n2.z));
    // Here, it is done per axis so that the terrain normal is swizzled into that tangent space...
    // (Unsure why abs is used, from what I could see I got the same regardless, 
    // but I kept it as the bgolus article used it)

    vec3 tangentX = vec3(nX.xy + terrainNormal.zy, abs(nX.z) * terrainNormal.x);
    vec3 tangentY = vec3(nY.xy + terrainNormal.xz, abs(nY.z) * terrainNormal.y);
    vec3 tangentZ = vec3(nZ.xy + terrainNormal.xy, abs(nZ.z) * terrainNormal.z);

    // ...then the result is swizzled back to world space.
    vec3 worldX = tangentX.zyx;
    vec3 worldY = tangentY.xzy;
    // NOTE: .xyz is just here for consistency.
    vec3 worldZ = tangentZ.xyz;

    return normalize(worldX * weights.x + worldY * weights.y + worldZ * weights.z);
}

void main()
{
    ivec2 terrainSize = textureSize(heightMap, 0);
    // The terrain normal, which is given in world space, so y is up.
    vec3 terrainNormal = useNeighbours ? neighbourNormal(texCoord, 1.0/terrainSize.x, 1.0/terrainSize.y, heightScale) : positionNormal(positionWithHeight);

    float slope = 1.0 - terrainNormal.y;

    vec3 absNormal = abs(terrainNormal);
    // Higher values give sharper transitions
    vec3 blendWeights = pow(absNormal, vec3(triplanarBlendFactor));
    // normalize the sum to 1 (not the actual vector).
    // Using dot products works as an optimisation
    blendWeights /= dot(blendWeights, vec3(1.0));

    vec2 uvX = positionWithHeight.zy * textureZoom;
    vec2 uvY = positionWithHeight.xz * textureZoom;
    vec2 uvZ = positionWithHeight.xy * textureZoom;

    vec3 grass = triplanarTexture(grassTexture, uvX, uvY, uvZ, blendWeights);
    vec3 rock = triplanarTexture(rockTexture, uvX, uvY, uvZ, blendWeights);
    vec3 sand = triplanarTexture(sandTexture, uvX, uvY, uvZ, blendWeights);
    vec3 snow = triplanarTexture(snowTexture, uvX, uvY, uvZ, blendWeights);

    // TODO: make dependent on water level (in a relative way)
    // TODO: customise
    float snowHeightBlend = smoothstep(70.0, 70.0 + 5.0, positionWithHeight.y);
    float snowSlopeFactor = 1.0 - smoothstep(0.0, sandThreshold, slope);
    float snowBlend = snowHeightBlend * snowSlopeFactor;

    // TODO: update name
    float heightBlend = 1.0 - smoothstep(waterLevel, waterLevel + sandLevelOffset, positionWithHeight.y);
    float slopeFactor = 1.0 - smoothstep(0.0, sandThreshold, slope);
    float sandBlend = heightBlend * slopeFactor;

    float rockBlend = smoothstep(grassThreshold, rockThreshold, slope);
    vec3 colour = mix(grass, rock, rockBlend);
    colour = mix(colour, snow, snowBlend);
    colour = mix(colour, sand, sandBlend);

    vec3 grassNormal = triplanarNormal(grassNormalMap, uvX, uvY, uvZ, blendWeights, terrainNormal);
    vec3 rockNormal  = triplanarNormal(rockNormalMap,  uvX, uvY, uvZ, blendWeights, terrainNormal);
    vec3 sandNormal  = triplanarNormal(sandNormalMap,  uvX, uvY, uvZ, blendWeights, terrainNormal);
    vec3 snowNormal  = triplanarNormal(snowNormalMap,  uvX, uvY, uvZ, blendWeights, terrainNormal);

    // Make sure they are blended like the colour textures.
    vec3 finalNormal = normalize(mix(grassNormal, rockNormal, rockBlend));
    finalNormal = normalize(mix(finalNormal, snowNormal, snowBlend));
    finalNormal = normalize(mix(finalNormal, sandNormal, sandBlend));

    // Simple shading with some ambient and mostly diffuse
    float diffuse = max(dot(finalNormal, normalize(sunDirection)), 0.0);
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(colour * light, 1.0);
}

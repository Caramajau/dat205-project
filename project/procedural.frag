#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform float heightScale;
uniform float gridSize;
uniform sampler2D heightMap;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D grassNormalMap;
uniform sampler2D rockNormalMap;
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

// Build a TBN matrix to rotate tangent space normals into world space.
mat3 buildTBN(vec3 terrainNormal)
{
    // For a heightmap terrain the tangent is just positive X
    vec3 T = vec3(1.0, 0.0, 0.0);
    // Performing Gram-Schmidt to re-orthogonalise.
    // T = normalize(T - dot(T, N) * N)
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - terrainNormal.x * terrainNormal);

    // Similarly, the bitangent is just positive Z
    // Maybe cross product?
    vec3 B = vec3(0.0, 0.0, 1.0);
    B = normalize(B - terrainNormal * terrainNormal.z);

    return mat3(T, B, terrainNormal);
}

// Whiteout blend (e.g. shouldn't do it linearly)
// Different approaches explained in: https://blog.selfshadow.com/publications/blending-in-detail/
// NOTE: the blending is done in tangent space
vec3 blendNormals(vec3 base, vec3 detail)
{
    return normalize(vec3(base.xy + detail.xy, base.z * detail.z));
}

// Unpack into a [-1,1] tangent space vector.
vec3 unpackNormal(sampler2D map, vec2 uv)
{
    return texture(map, uv).rgb * 2.0 - 1.0;
}

void main()
{
    // The terrain normal, which is given in world space, so y is up.
    vec3 terrainNormal = useNeighbours ? neighbourNormal(texCoord, 1.0/gridSize, heightScale) : positionNormal(positionWithHeight);

    float slope = 1.0 - terrainNormal.y;

    // 32 seemed like a good "zoom".
    vec2 textureUV = 32.0 * texCoord;

    // Texture from https://ambientcg.com/a/Grass005
    vec3 grass = texture(grassTexture, textureUV).rgb;
    // Texture from https://ambientcg.com/a/Ground067
    vec3 rock = texture(rockTexture, textureUV).rgb;

    float rockBlend = smoothstep(0.2, 0.4, slope);
    vec3 colour = mix(grass, rock, rockBlend);

    // Normals from textures, given in tangent space, z is up (which is why they are blueish).
    vec3 grassNormal = unpackNormal(grassNormalMap, textureUV);
    vec3 rockNormal = unpackNormal(rockNormalMap, textureUV);

    // Make sure they are blended like the colour textures.
    vec3 detailNormal = normalize(mix(grassNormal, rockNormal, rockBlend));

    // Construct TBN via terrain normal so can convert back.
    mat3 TBN = buildTBN(terrainNormal);
    // NOTE: In tangent space the terrain normal is just (0,0,1).
    vec3 blended = blendNormals(vec3(0.0, 0.0, 1.0), detailNormal);
    vec3 finalNormal = normalize(TBN * blended);

    // Simple shading with some ambient and mostly diffuse
    float diffuse = max(dot(finalNormal, normalize(sunDirection)), 0.0);
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(colour * light, 1.0);
}

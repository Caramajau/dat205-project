#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;

uniform float heightScale;
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
vec3 neighbourNormal(vec2 uv, float texelWidth, float texelHeight, float texelAspect)
{
    float hD = texture(heightMap, uv + texelHeight*vec2( 0,-1)).r * texelAspect;
    float hL = texture(heightMap, uv + texelWidth*vec2(-1, 0)).r * texelAspect;
    float hR = texture(heightMap, uv + texelWidth*vec2( 1, 0)).r * texelAspect;
    float hU = texture(heightMap, uv + texelHeight*vec2( 0, 1)).r * texelAspect;

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

    // Can get the bitangent with cross product.
    // Although, unlike in the article linked above this is the correct order.
    // They might be dealing with reversed x axis or something.
    vec3 B = cross(T, terrainNormal);

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

vec3 triplanarTexture(sampler2D tex, vec2 uvX, vec2 uvY, vec2 uvZ, vec3 weights)
{
    vec3 texX = texture(tex, uvX).rgb;
    vec3 texY = texture(tex, uvY).rgb;
    vec3 texZ = texture(tex, uvZ).rgb;

    return texX * weights.x + texY * weights.y + texZ * weights.z;
}

vec3 triplanarNormal(sampler2D map, vec2 uvX, vec2 uvY, vec2 uvZ, vec3 weights, vec3 terrainNormal)
{
    vec3 nX = unpackNormal(map, uvX);
    vec3 nY = unpackNormal(map, uvY);
    vec3 nZ = unpackNormal(map, uvZ);

    // Whiteout blend each slab with the terrain normal swizzled into that slab's tangent space,
    // then swizzle the result back to world space
    vec3 worldX = vec3(nX.xy + terrainNormal.zy, abs(nX.z) * terrainNormal.x).zyx;
    vec3 worldY = vec3(nY.xy + terrainNormal.xz, abs(nY.z) * terrainNormal.y).xzy;
    vec3 worldZ = vec3(nZ.xy + terrainNormal.xy, abs(nZ.z) * terrainNormal.z).xyz;

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
    vec3 blendWeights = pow(absNormal, vec3(4.0));
    // normalize the sum to 1 (not the actual vector).
    // Using dot products works as an optimisation
    blendWeights /= dot(blendWeights, vec3(1.0));

    // 0.1 felt like a good "zoom".
    float scale = 0.1;
    vec2 uvX = positionWithHeight.zy * scale;
    vec2 uvY = positionWithHeight.xz * scale;
    vec2 uvZ = positionWithHeight.xy * scale;

    // Texture from https://ambientcg.com/a/Grass005
    vec3 grass = triplanarTexture(grassTexture, uvX, uvY, uvZ, blendWeights);

    // Texture from https://ambientcg.com/a/Ground067
    vec3 rock = triplanarTexture(rockTexture, uvX, uvY, uvZ, blendWeights);

    float rockBlend = smoothstep(0.2, 0.4, slope);
    vec3 colour = mix(grass, rock, rockBlend);

    vec3 grassNormal = triplanarNormal(grassNormalMap, uvX, uvY, uvZ, blendWeights, terrainNormal);
    vec3 rockNormal  = triplanarNormal(rockNormalMap,  uvX, uvY, uvZ, blendWeights, terrainNormal);

    // Make sure they are blended like the colour textures.
    vec3 finalNormal = normalize(mix(grassNormal, rockNormal, rockBlend));

    // Simple shading with some ambient and mostly diffuse
    float diffuse = max(dot(finalNormal, normalize(sunDirection)), 0.0);
    float light = 0.1 + 0.9 * diffuse;

    fragmentColor = vec4(colour * light, 1.0);
}

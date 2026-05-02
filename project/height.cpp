#include "height.h"

std::vector<float> createHeightMap(const ProceduralConfig& config) {
    std::vector<float> grid(config.width * config.height);

    InterpolateFunc interpolate = convertTypeToMethodInterpolationType(config.interpolationType);
    InterpolateFunc interpolateDerivative = convertTypeToMethodDerivativeType(config.interpolationType);
    ErosionFunc erosion = convertTypeToMethodErosionType(config.erosionType);

    auto fbm = FbmNoise(config.seed, config.octaveCount, config.lacunarity, config.persistence, interpolate, interpolateDerivative, erosion, config.erosionStrength);

    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            float fx = (float)x / config.gridSize;
            float fy = (float)y / config.gridSize;

            if (config.warpLevel != 0) {
                domainWarp(fbm, fx, fy, config.warpLevel);
            }

            grid[y * config.width + x] = fbm.sample(fx, fy);
        }
    }
    return grid;
}

// Domain warping based on: https://iquilezles.org/articles/warp/
void domainWarp(FbmNoise& fbm, float& fx, float& fy, int warpLevel)
{
    // First warping
    float qx = 4 * fbm.sample(fx, fy);
    float qy = 4 * fbm.sample(fx + 5.2f, fy + 1.3f);

    if (warpLevel == 1) {
        fx += qx;
        fy += qy;
    }
    else if (warpLevel == 2) {
        // Second warping
        float rx = 4 * fbm.sample(qx + 1.7f, qy + 9.2f);
        float ry = 4 * fbm.sample(qx + 8.3f, qy + 2.8f);

        fx += rx;
        fy += ry;
    }
}

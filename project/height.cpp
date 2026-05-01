#include "height.h"

std::vector<float> createHeightMap(int seed, int width, int height, int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType, int warpLevel) {
    std::vector<float> grid(width * height);

    InterpolateFunc interpolate = convertTypeToMethodInterpolationType(interpolationType);
    InterpolateFunc interpolateDerivative = convertTypeToMethodDerivativeType(interpolationType);

    auto fbm = FbmNoise(seed, octaveCount, lacunarity, persistence, interpolate, interpolateDerivative);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float fx = (float)x / gridSize;
            float fy = (float)y / gridSize;

            // TODO: Make customisable through GUI?

            // Domain warping based on: https://iquilezles.org/articles/warp/

            if (warpLevel == 1 || warpLevel == 2) {
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

            grid[y * width + x] = fbm.sample(fx, fy);
        }
    }
    return grid;
}

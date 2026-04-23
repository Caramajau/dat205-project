#include "height.h"

std::vector<float> createHeightMap(int seed, int width, int height, int gridSize, int octaveCount, float lacunarity, float persistence, InterpolationType interpolationType) {
    std::vector<float> grid(width * height);

    InterpolateFunc interpolate = convertTypeToMethodInterpolationType(interpolationType);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float fx = (float)x / gridSize;
            float fy = (float)y / gridSize;

            // TODO: Make customisable through GUI?

            // Domain warping based on: https://iquilezles.org/articles/warp/

            // First warping
            float qx = 4 * fbm(octaveCount, seed, fx, fy, interpolate, lacunarity, persistence);
            float qy = 4 * fbm(octaveCount, seed, fx + 5.2f, fy + 1.3f, interpolate, lacunarity, persistence);

            // Second warping
            float rx = 4 * fbm(octaveCount, seed, qx + 1.7f, qy + 9.2f, interpolate, lacunarity, persistence);
            float ry = 4 * fbm(octaveCount, seed, qx + 8.3f, qy + 2.8f, interpolate, lacunarity, persistence);

            // fx += qx;
            // fy += qy;

            fx += rx;
            fy += ry;

            grid[y * width + x] = fbm(octaveCount, seed, fx, fy, interpolate, lacunarity, persistence);
        }
    }
    return grid;
}

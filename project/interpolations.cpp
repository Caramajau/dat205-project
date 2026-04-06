#include "interpolations.h"

// Incorrect version of cubic interpolation, which is what I originally did when I followed the perlin video.
// It is basically linear interpolation, but with extra steps..., maybe it will be interesting to look at?
float incorrectCubicInterpolation(float firstValue, float secondValue, float weight) {
    return (firstValue - secondValue) * (3.0f - weight * 2.0f) * weight * weight + firstValue;
}

// Correct version of cubic interpolation, the interpolation suggested in the perlin video.
float cubicInterpolate(float a, float b, float weight) {
    float t = weight * weight * (3.0f - 2.0f * weight);
    return a + (b - a) * t;
}

// NOTE: This function is based on the formula from this video (quintic interpolation):
// https://www.youtube.com/watch?v=ZsEnnB2wrbI
float quinticInterpolate(float a, float b, float weight) {
    float t = weight * weight * weight * (weight * (weight * 6.0f - 15.0f) + 10.0f);
    return a + (b - a) * t;
}

InterpolateFunc convertTypeToMethodInterpolationType(InterpolationType interpolationType) {
	switch (interpolationType) {
		case InterpolationType::Incorrect: return incorrectCubicInterpolation;
		case InterpolationType::Cubic: return cubicInterpolate;
		case InterpolationType::Quintic: return quinticInterpolate;
		default: return nullptr;
	}
}

#include "interpolations.h"

float linearInterpolate(float weight) {
	return weight;
}

float linearDerivative(float) {
	return 1.0f;
}

// Correct version of cubic interpolation, the interpolation suggested in the perlin video.
float cubicInterpolate(float weight) {
    return weight * weight * (3.0f - 2.0f * weight);
}

float cubicDerivative(float weight)
{
	return 6.0f * weight * (1.0f - weight);
}

// NOTE: This function is based on the formula from this video (quintic interpolation):
// https://www.youtube.com/watch?v=ZsEnnB2wrbI
float quinticInterpolate(float weight) {
    return weight * weight * weight * (weight * (weight * 6.0f - 15.0f) + 10.0f);
}

float quinticDerivative(float weight)
{
	return weight * weight * (weight * (weight * 30.0f - 60.0f) + 30.0f);
}

float blending(float a, float b, float blendingFactor) {
	// blendingFactor * b + (1 - blendingFactor) * a
	return a + (b - a) * blendingFactor;
}

// When I initially wrote the cubic interpolation, I messed up the order for the blending.
// This version is kept, since maybe it will be interesting to look at?
float incorrectBlending(float a, float b, float blendingFactor) {
	return a + (a - b) * blendingFactor;
}

InterpolateFunc convertTypeToMethodInterpolationType(InterpolationType interpolationType) {
	switch (interpolationType) {
		case InterpolationType::Linear: return &linearInterpolate;
		case InterpolationType::Cubic: return &cubicInterpolate;
		case InterpolationType::Quintic: return &quinticInterpolate;
		default: return nullptr;
	}
}

InterpolateFunc convertTypeToMethodDerivativeType(InterpolationType interpolationType) {
	switch (interpolationType) {
		case InterpolationType::Linear: return &linearDerivative;
		case InterpolationType::Cubic: return &cubicDerivative;
		case InterpolationType::Quintic: return &quinticDerivative;
		default: return nullptr;
	}
}

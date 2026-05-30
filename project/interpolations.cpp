#include "interpolations.h"

float noSmooth(float weight) {
	return weight;
}

float noSmoothDerivative(float) {
	return 1.0f;
}

// Correct version of cubic interpolation AKA smooth step, the interpolation suggested in the perlin video.
float smoothStep(float weight) {
    return weight * weight * (3.0f - 2.0f * weight);
}

float smoothStepDerivative(float weight)
{
	return 6.0f * weight * (1.0f - weight);
}

// NOTE: This function is based on the formula from this video (quintic interpolation AKA smoother step):
// https://www.youtube.com/watch?v=ZsEnnB2wrbI
float smootherStep(float weight) {
    return weight * weight * weight * (weight * (weight * 6.0f - 15.0f) + 10.0f);
}

float smootherStepDerivative(float weight)
{
	return weight * weight * (weight * (weight * 30.0f - 60.0f) + 30.0f);
}

float lerp(float a, float b, float blendingFactor) {
	// blendingFactor * b + (1 - blendingFactor) * a
	return a + (b - a) * blendingFactor;
}

// When I initially wrote the cubic interpolation, I messed up the order for the lerp.
// This version is kept, since maybe it will be interesting to look at?
float incorrectLerp(float a, float b, float blendingFactor) {
	return a + (a - b) * blendingFactor;
}

SmoothFunc convertTypeToMethodSmoothType(SmoothType smoothType) {
	switch (smoothType) {
		case SmoothType::NoSmooth: return &noSmooth;
		case SmoothType::SmoothStep: return &smoothStep;
		case SmoothType::SmootherStep: return &smootherStep;
		default: return nullptr;
	}
}

SmoothFunc convertTypeToMethodDerivativeType(SmoothType smoothType) {
	switch (smoothType) {
		case SmoothType::NoSmooth: return &noSmoothDerivative;
		case SmoothType::SmoothStep: return &smoothStepDerivative;
		case SmoothType::SmootherStep: return &smootherStepDerivative;
		default: return nullptr;
	}
}

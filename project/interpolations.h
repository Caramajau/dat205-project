#pragma once

// Here are functions that can be used for the smoothing as well as the lerp.

// Here mostly to make it used similar to other methods.
float noSmooth(float weight);
float noSmoothDerivative(float);

float smoothStep(float weight);
float smoothStepDerivative(float weight);

float smootherStep(float weight);
float smootherStepDerivative(float weight);

// Will interpolate (blend) between the first and second value, blendingFactor between 0 and 1.
float lerp(float a, float b, float blendingFactor);

float incorrectLerp(float a, float b, float blendingFactor);

enum class SmoothType
{
	NoSmooth,
	SmoothStep,
	SmootherStep
};

// Function pointer for what kind of interpolation function to use.
using SmoothFunc = float(*)(float);

SmoothFunc convertTypeToMethodSmoothType(SmoothType smoothType);
SmoothFunc convertTypeToMethodDerivativeType(SmoothType smoothType);

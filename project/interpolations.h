#pragma once

// These will interpolate between the first and second value, weight between 0 and 1.

float incorrectCubicInterpolation(float weight);

float cubicInterpolate(float weight);

float quinticInterpolate(float weight);

float blending(float a, float b, float blendingFactor);

float incorrectBlending(float a, float b, float blendingFactor);

enum class InterpolationType
{
	Incorrect,
	Cubic,
	Quintic
};

// Function pointer for what kind of interpolation function to use.
using InterpolateFunc = float(*)(float);

InterpolateFunc convertTypeToMethodInterpolationType(InterpolationType interpolationType);

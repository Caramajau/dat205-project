#pragma once

// Here are functions that can be used for the interpolations as well as the actual blending.

// Here mostly to make it used similar to other methods.
float linearInterpolate(float weight);

float cubicInterpolate(float weight);

float quinticInterpolate(float weight);

// Will interpolate (blend) between the first and second value, blendingFactor between 0 and 1.
float blending(float a, float b, float blendingFactor);

float incorrectBlending(float a, float b, float blendingFactor);

enum class InterpolationType
{
	Linear,
	Cubic,
	Quintic
};

// Function pointer for what kind of interpolation function to use.
using InterpolateFunc = float(*)(float);

InterpolateFunc convertTypeToMethodInterpolationType(InterpolationType interpolationType);

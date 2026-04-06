#pragma once

// These will interpolate between the first and second value, weight between 0 and 1.

float incorrectCubicInterpolation(float firstValue, float secondValue, float weight);

float cubicInterpolate(float a, float b, float weight);

float quinticInterpolate(float a, float b, float weight);

enum class InterpolationType
{
	Incorrect,
	Cubic,
	Quintic
};

// Function pointer for what kind of interpolation function to use.
using InterpolateFunc = float(*)(float, float, float);

InterpolateFunc convertTypeToMethodInterpolationType(InterpolationType interpolationType);

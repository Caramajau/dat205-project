#pragma once
#include <math.h>

enum class ErosionType
{
	None,
	Rational,
	Exponential
};

float noErosion(float, float);

float rationalErosion(float length, float strength);

float exponentialErosion(float length, float strength);

// Function pointer for what kind of erosion function to use.
using ErosionFunc = float(*)(float, float);

ErosionFunc convertTypeToMethodErosionType(ErosionType erosionType);

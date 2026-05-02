#pragma once
#include <math.h>

enum class ErosionType
{
	Rational,
	Exponential
};

float rationalErosion(float length, float strength);

float exponentialErosion(float length, float strength);

// Function pointer for what kind of erosion function to use.
using ErosionFunc = float(*)(float, float);

ErosionFunc convertTypeToMethodErosionType(ErosionType erosionType);

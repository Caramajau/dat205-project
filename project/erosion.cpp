#include "erosion.h"


float noErosion(float, float) {
	return 1.0f;
}

float rationalErosion(float length, float strength) {
	return 1.0f / (1.0f + strength * length);
}

float exponentialErosion(float length, float strength) {
	return expf(-strength * length);
}

ErosionFunc convertTypeToMethodErosionType(ErosionType erosionType) {
	switch (erosionType) {
		case ErosionType::None: return &noErosion;
		case ErosionType::Rational: return &rationalErosion;
		case ErosionType::Exponential: return &exponentialErosion;
		default: return nullptr;
	}
}

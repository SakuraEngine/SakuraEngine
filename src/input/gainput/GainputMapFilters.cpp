
#include "input/gainput/gainput.h"
#include "input/gainput/GainputMapFilters.h"


namespace gainput
{

float InvertSymmetricInput(float const value, void*)
{
	return -value;
}

float InvertInput(float const value, void*)
{
	return 1.0f - value;
}

}


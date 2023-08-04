
#pragma once

#include "basic_types.h"
#include <limits>

namespace ostr::global_constant
{
	static constexpr u64 INDEX_INVALID = std::numeric_limits<u64>::max();
	static constexpr u64 SIZE_INVALID = std::numeric_limits<u64>::max();
	
	static constexpr u64 TOLERANCE_EXPONENT = 3;
	static constexpr f64 TOLERANCE = 1e-3;
	
	static constexpr f64 GOLDEN_RATIO = 1.61803398874989484820458683436563811772030917980576286213544862270526046281890;
}

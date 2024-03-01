#pragma once
#include "OpenString/common/definitions.h"

namespace ostr
{
	using i8 = int8_t;
	using u8 = uint8_t;
	using i16 = int16_t;
	using u16 = uint16_t;
	using i32 = int32_t;
	using u32 = uint32_t;
	using i64 = int64_t;
	using u64 = uint64_t;

	using f32 = float;
	using f64 = double;

	using byte = u8;
}

inline namespace literal
{
	[[nodiscard]] constexpr ochar8_t operator""_as_char(const unsigned long long value) noexcept
	{
		return static_cast<ochar8_t>(value);
	}
}

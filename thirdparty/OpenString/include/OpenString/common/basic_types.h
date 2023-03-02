// OpenString - basic types
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once
#include "definitions.h"

OPEN_STRING_NS_BEGIN

using i8 = signed char;
using u8 = unsigned char;
using i16 = signed short;
using u16 = unsigned short;
using i32 = signed int;
using u32 = unsigned int;
using i64 = signed long long;
using u64 = unsigned long long;

inline constexpr i32 index_invalid = -1;

OPEN_STRING_NS_END

inline namespace literal
{
	[[nodiscard]] constexpr char operator""_as_char(const unsigned long long value) noexcept
	{
		return static_cast<char>(value);
	}
}

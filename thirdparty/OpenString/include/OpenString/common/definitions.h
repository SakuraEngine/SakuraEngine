// OpenString - definitions
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.
#pragma once
#include "platform/configure.h"

#ifndef OPEN_STRING_NS
#define OPEN_STRING_NS easy
#endif

#ifndef OPEN_STRING_NS_BEGIN
#define OPEN_STRING_NS_BEGIN	namespace OPEN_STRING_NS {
#endif

#ifndef OPEN_STRING_NS_END
#define OPEN_STRING_NS_END		}
#endif

#ifndef OPEN_STRING_API
#define OPEN_STRING_API
#endif

#if __cplusplus >= 201100L
#define OSTR_UTF8(str) u8##str
#else
#define OSTR_UTF8(str) str
#endif

using ochar_t = char;
using ochar8_t = char8_t; // already defined in platform/configure.h
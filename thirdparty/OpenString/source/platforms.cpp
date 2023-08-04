// OpenString - human-readable string
//
// Copyright (c) 2023 - present, [Hoshizora Ming]
// All rights reserved.
#include "OpenString/common/definitions.h"

namespace ostr 
{
    const ochar8_t* kOpenStringMemory = u8"OpenString";
}

#if _WIN64

#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <memory.h>
#include <intrin.h>
#include <Windows.h>
#include <debugapi.h>


namespace ostr
{
void PlatformReportError(const ochar8_t* str)
{
	OutputDebugStringA((const char*)str);
}
}

#else
#include "stdio.h"

namespace ostr
{
void PlatformReportError(const ochar8_t* str)
{
    printf("%s\n", str);
}
}

#endif

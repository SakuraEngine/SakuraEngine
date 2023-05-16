// OpenString - human-readable string
//
// Copyright (c) 2023 - present, [Hoshizora Ming]
// All rights reserved.

#if _WIN64

#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <memory.h>
#include "text.h"
#include "adapters.h"

#include <intrin.h>
#include <Windows.h>
#include <debugapi.h>

OPEN_STRING_NS_BEGIN

void PlatformReportError(const char* str)
{
	OutputDebugStringA(str);
}

OPEN_STRING_NS_END

#else

OPEN_STRING_NS_BEGIN

void PlatformReportError(const char* str)
{
}

OPEN_STRING_NS_END

#endif

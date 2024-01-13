
#pragma once
#include "OpenString/common/definitions.h"

namespace ostr
{
	OPEN_STRING_API void PlatformReportError(const char8_t* str);
}

#if _WIN64
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif

	#ifndef NOMINMAX
	#define NOMINMAX
	#endif

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif

	#define OPEN_STRING_PRINT_SIMPLE_DEBUG_MESSAGE(...)	\
	{	\
		[](const ochar8_t* message_format, auto&&...args)	\
		{	\
			const ostr::codeunit_sequence message = ostr::format(message_format, args...);	\
			ostr::PlatformReportError(message.u8_str());	\
		}(__VA_ARGS__);	\
	}

	#define OPEN_STRING_PRINT_FORMATTED_DEBUG_MESSAGE(...)	\
	{	\
		[](const ochar8_t* debug_format, const ochar8_t* message_format, auto&&...args)	\
		{	\
			const ostr::codeunit_sequence message = ostr::format(message_format, args...);	\
			ostr::PlatformReportError(ostr::format(debug_format, message).u8_str());	\
		}(__VA_ARGS__);	\
	}

    // Visual Studio will not trigger the breakpoint during single-step debugging without __nop()
    #define OPEN_STRING_DEBUG_BREAK() __debugbreak()

#elif defined(__linux__) || defined(__MACH__)

	#define OPEN_STRING_PRINT_SIMPLE_DEBUG_MESSAGE(...)
	#define OPEN_STRING_PRINT_FORMATTED_DEBUG_MESSAGE(...)
	#define OPEN_STRING_DEBUG_BREAK()

#endif

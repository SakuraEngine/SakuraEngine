// OpenString - definitions
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.
#pragma once

#include "platform/configure.h"
#define OSTR_USE_CXX20_CHAR8_TYPE

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

#ifndef OPEN_STRING_STRINGIFY
#define OPEN_STRING_STRINGIFY(x) #x
#endif
#ifndef OPEN_STRING_STRINGIFY_EXPANDED
#define OPEN_STRING_STRINGIFY_EXPANDED(x) OPEN_STRING_STRINGIFY(x)
#endif

#if __cpp_char8_t

    #ifdef OSTR_USE_CXX20_CHAR8_TYPE
    #ifndef OCHAR8_TYPE
        #define OCHAR8_TYPE char8_t
    #endif
    #define OSTR_UTF8(str) u8##str

    #else

    #ifndef OCHAR8_TYPE
        #define OCHAR8_TYPE char
    #endif
    #define OSTR_UTF8(str) str

    #endif
#endif

#ifndef OSTR_UTF8
    #if __cplusplus >= 201100L
    #define OSTR_UTF8(str) u8##str
    #else
    #define OSTR_UTF8(str) str
    #endif
#endif

#ifndef OCHAR_TYPE
#define OCHAR_TYPE char
#endif

#ifndef OCHAR8_TYPE
#define OCHAR8_TYPE char
#endif

using ochar_t = OCHAR_TYPE;
using ochar8_t = OCHAR8_TYPE; 

OPEN_STRING_NS_BEGIN
OPEN_STRING_API void PlatformReportError(const char* str);
OPEN_STRING_NS_END

#if _WIN64
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif

    #define OPEN_STRING_PRINT_DEBUG_MESSAGE(...)	\
    {	\
        [](auto debug_format, auto message_format, auto&&...args)	\
        {	\
            if constexpr (sizeof...(args)) \
            {\
                const OPEN_STRING_NS::codeunit_sequence message = OPEN_STRING_NS::format(message_format, args...);	\
                PlatformReportError(OPEN_STRING_NS::format(debug_format, message).c_str());	\
            } \
            else \
            {\
                PlatformReportError(OPEN_STRING_NS::format(debug_format, message_format).c_str());	\
            }\
        }(__VA_ARGS__);	\
    }

    // Visual Studio will not trigger the breakpoint during single-step debugging without __nop()
    #include <intrin.h>
    #define OPEN_STRING_DEBUG_BREAK() (__nop(), __debugbreak())
#endif

#ifndef OPEN_STRING_UNLIKELY
#define OPEN_STRING_UNLIKELY(expression)    (!!(expression))
#endif

#ifndef OPEN_STRING_PRINT_DEBUG_MESSAGE
#define OPEN_STRING_PRINT_DEBUG_MESSAGE(...)
#endif

#ifndef OPEN_STRING_DEBUG_BREAK
#define OPEN_STRING_DEBUG_BREAK()
#endif

#ifndef OPEN_STRING_CHECK
#define OPEN_STRING_CHECK(expression, ...)  \
    { \
        if(OPEN_STRING_UNLIKELY(!(expression))) \
        { \
            OPEN_STRING_PRINT_DEBUG_MESSAGE(OSTR_UTF8("\nCheck failed: {}\n\t[" __FILE__ ":" OPEN_STRING_STRINGIFY_EXPANDED(__LINE__) "] " OPEN_STRING_STRINGIFY(expression) "\n"), __VA_ARGS__ ); \
            OPEN_STRING_DEBUG_BREAK(); \
        } \
    }
#endif

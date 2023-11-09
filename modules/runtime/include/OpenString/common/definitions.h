#pragma once
#include "SkrRT/config.h" // IWYU pragma: export

#ifndef OPEN_STRING_API
#define OPEN_STRING_API
#endif

#ifndef OPEN_STRING_CODE_BLOCK
#define OPEN_STRING_CODE_BLOCK(code) do{code}while(false)
#endif

#ifndef OPEN_STRING_STRINGIFY
#define OPEN_STRING_STRINGIFY(x) #x
#endif
#ifndef OPEN_STRING_STRINGIFY_EXPANDED
#define OPEN_STRING_STRINGIFY_EXPANDED(x) OPEN_STRING_STRINGIFY(x)
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
#ifdef OSTR_USE_CXX20_CHAR8_TYPE
#define OCHAR8_TYPE char8_t
#else
#define OCHAR8_TYPE char
#endif
#endif
using ochar_t = OCHAR_TYPE;
using ochar8_t = OCHAR8_TYPE; 

#ifndef OPEN_STRING_UNLIKELY
#define OPEN_STRING_UNLIKELY(expression)    (!!(expression))
#endif

#ifndef OPEN_STRING_CHECK_OR
#define OPEN_STRING_CHECK_OR(result, expression, ...)  \
	OPEN_STRING_CODE_BLOCK({ \
		if(OPEN_STRING_UNLIKELY(!(expression))) \
		{ \
			OPEN_STRING_PRINT_FORMATTED_DEBUG_MESSAGE(u8"\nCheck failed: {}\n\t[" __FILE__ ":" OPEN_STRING_STRINGIFY_EXPANDED(__LINE__) "] " OPEN_STRING_STRINGIFY(expression) "\n", __VA_ARGS__ ); \
			OPEN_STRING_DEBUG_BREAK(); \
			result; \
		} \
	})
#endif

#ifndef OPEN_STRING_CHECK
#define OPEN_STRING_CHECK(expression, ...)  OPEN_STRING_CHECK_OR(/* Do nothing */, expression, __VA_ARGS__)
#endif

#ifndef OPEN_STRING_RESULT_CHECK
#define OPEN_STRING_RESULT_CHECK(result) OPEN_STRING_CODE_BLOCK(	\
		HRESULT result__{ result };	\
		OPEN_STRING_CHECK(SUCCEEDED(result__), "Result check failed! {}", _com_error{ result__ }.ErrorMessage());	\
	)
#endif
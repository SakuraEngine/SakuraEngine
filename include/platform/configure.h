#pragma once

#ifndef RUNTIME_EXPORT
    #if defined(_MSC_VER)
        #define RUNTIME_EXPORT __declspec(dllexport)
    #else
        #define RUNTIME_EXPORT
    #endif
#endif

#ifndef RUNTIME_API // If the build file hasn't already defined this to be dllexport...
	#ifdef RUNTIME_DLL
		#if defined(_MSC_VER)
			#define RUNTIME_API      __declspec(dllimport)
			#define RUNTIME_LOCAL
		#elif defined(__CYGWIN__)
			#define RUNTIME_API      __attribute__((dllimport))
			#define RUNTIME_LOCAL
		#elif (defined(__GNUC__) && (__GNUC__ >= 4))
			#define RUNTIME_API      __attribute__ ((visibility("default")))
			#define RUNTIME_LOCAL    __attribute__ ((visibility("hidden")))
		#else
			#define RUNTIME_API
			#define RUNTIME_LOCAL
		#endif

		#define EA_DLL

	#else
		#define RUNTIME_API
		#define RUNTIME_LOCAL
	#endif
#endif

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #define _MACOS
    #endif
#elif defined _WIN32 || defined _WIN64
#endif 
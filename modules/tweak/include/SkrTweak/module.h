#pragma once
#include "SkrTweak/module.configure.h"
#include "module/module.hpp"

#ifdef __cplusplus
struct skr_tweak_int_t;
struct skr_tweak_float_t;
struct skr_tweak_bool_t;
struct skr_tweak_string_t;
SKR_TWEAK_API skr_tweak_int_t* skr_tweak_value(int value, const char* str, const char* fileName, int lineNumber);
SKR_TWEAK_API int skr_get_tweak(skr_tweak_int_t* tweak);
SKR_TWEAK_API skr_tweak_float_t* skr_tweak_value(float value, const char* str, const char* fileName, int lineNumber);
SKR_TWEAK_API float skr_get_tweak(skr_tweak_float_t* tweak);
SKR_TWEAK_API skr_tweak_bool_t* skr_tweak_value(bool value, const char* str, const char* fileName, int lineNumber);
SKR_TWEAK_API bool skr_get_tweak(skr_tweak_bool_t* tweak);
SKR_TWEAK_API skr_tweak_string_t* skr_tweak_value(const char* value, const char* str, const char* fileName, int lineNumber);
SKR_TWEAK_API const char* skr_get_tweak(skr_tweak_string_t* tweak);

#if !defined(SKR_SHIPPING) && !defined(SHIPPING_ONE_ARCHIVE)
#define SKR_TWEAK(value) []() { static auto tweak = skr_tweak_value((value), #value, __FILE__, __LINE__); return skr_get_tweak(tweak); }()
#define TWEAK_USABLE
#else
#define SKR_TWEAK(value) (value)
#endif
#endif
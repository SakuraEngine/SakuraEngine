#pragma once
#include "SkrInspector/module.configure.h"

namespace skr::inspect
{

#if !defined(SKR_SHIPPING) && !defined(SHIPPING_ONE_ARCHIVE)
    #define SKR_INSPECT(value) [](auto currValue) \
     { static auto tweak = ::skr::inspect::add_inspected_value(currValue, #value, __FILE__, __LINE__);  return ::skr::inspect::update_inspected_value(tweak, currValue); }((value))
    #define INSPECT_USABLE
#else
    #define SKR_INSPECT(value) (value)
#endif

#ifdef INSPECT_USABLE
struct inspected_int;
struct inspected_float;
struct inspected_bool;
struct inspected_string;
SKR_INSPECT_API inspected_int* add_inspected_value(int value, const char* name, const char* file, int line);
SKR_INSPECT_API inspected_float* add_inspected_value(float value, const char* name, const char* file, int line);
SKR_INSPECT_API inspected_bool* add_inspected_value(bool value, const char* name, const char* file, int line);
SKR_INSPECT_API inspected_string* add_inspected_value(const char* value, const char* name, const char* file, int line);

SKR_INSPECT_API int update_inspected_value(inspected_int* tweak, int value);
SKR_INSPECT_API float update_inspected_value(inspected_float* tweak, float value);
SKR_INSPECT_API bool update_inspected_value(inspected_bool* tweak, bool value);
SKR_INSPECT_API const char* update_inspected_value(inspected_string* tweak, const char* value);

SKR_INSPECT_API void update_value_inspector();
#else
inline void update_value_inspector() {}    
#endif
} // namespace skr::inspect
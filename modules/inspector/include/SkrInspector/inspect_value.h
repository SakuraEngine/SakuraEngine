#pragma once
#include "SkrInspector/module.configure.h"
#include "type/type.hpp"

namespace skr::inspect
{
#if !defined(SKR_SHIPPING) && !defined(SHIPPING_ONE_ARCHIVE)
    #define SKR_INSPECT(value) [](auto& currValue) -> decltype(currValue) \
     { static auto tweak = ::skr::inspect::add_inspected_value(currValue, #value, __FILE__, __LINE__);  ::skr::inspect::update_inspected_value(tweak, &currValue); return currValue; }((value))
    #define INSPECT_USABLE
#else
    #define SKR_INSPECT(value) (value)
#endif

#ifdef INSPECT_USABLE
struct inspected_object;
SKR_INSPECT_API inspected_object* add_inspected_object(void* data, const skr_type_t* type, const char* name, const char* file, int line);
template<class T>
inspected_object* add_inspected_value(T& data, const char* name, const char* file, int line)
{
    return add_inspected_object(&data, skr::type::type_of<T>::get(), name, file, line);
}

SKR_INSPECT_API void update_inspected_value(inspected_object* tweak, void* value);

SKR_INSPECT_API void update_value_inspector();
#else
inline void update_value_inspector() {}    
#endif
} // namespace skr::inspect
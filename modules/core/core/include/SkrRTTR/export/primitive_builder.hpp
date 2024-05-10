#pragma once
#include "SkrRTTR/export/export_data.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct PrimitiveBuilder {
    PrimitiveBuilder(PrimitiveData* data);

    // basic info
    PrimitiveBuilder& basic_info();

protected:
    PrimitiveData* _data;
};
} // namespace skr::rttr

namespace skr::rttr
{
template <typename T, typename Backend>
PrimitiveBuilder<T, Backend>::PrimitiveBuilder(PrimitiveData* data)
    : _data(data)
{
}

// basic info
template <typename T, typename Backend>
inline PrimitiveBuilder<T, Backend>& PrimitiveBuilder<T, Backend>::basic_info()
{
    // split namespace
    _data->name = RTTRTraits<T>::get_name();

    // fill type id
    _data->type_id = RTTRTraits<T>::get_guid();

    // fill size & alignment
    _data->size      = sizeof(T);
    _data->alignment = alignof(T);

    return *this;
}

} // namespace skr::rttr
#pragma once
#include "SkrRT/rttr/export/export_data.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct EnumBuilder {
    EnumBuilder(EnumData* data);

    // basic info
    EnumBuilder& name(String name);
    EnumBuilder& basic_info();

    // item
    EnumBuilder& item(String name, T value);

protected:
    EnumData* _data;
};
} // namespace skr::rttr

namespace skr::rttr
{
template <typename T, typename Backend>
EnumBuilder<T, Backend>::EnumBuilder(EnumData* data)
    : _data(data)
{
}

// basic info
template <typename T, typename Backend>
inline EnumBuilder<T, Backend>& EnumBuilder<T, Backend>::name(String name)
{
    // TODO. parse namesapce
    _data->name = std::move(name);
    return *this;
}
template <typename T, typename Backend>
inline EnumBuilder<T, Backend>& EnumBuilder<T, Backend>::basic_info()
{
    _data->type_id            = RTTRTraits<T>::get_guid();
    _data->underlying_type_id = RTTRTraits<std::underlying_type_t<T>>::get_guid();
    return *this;
}

// item
template <typename T, typename Backend>
inline EnumBuilder<T, Backend>& EnumBuilder<T, Backend>::item(String name, T value)
{
    _data->items.push_back({ std::move(name), EnumValue(static_cast<std::underlying_type_t<T>>(value)) });
    return *this;
}

} // namespace skr::rttr
#pragma once
#include "SkrRTTR/export/export_data.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct EnumBuilder {
    EnumBuilder(EnumData* data);

    // basic info
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
inline EnumBuilder<T, Backend>& EnumBuilder<T, Backend>::basic_info()
{
    // split namespace
    String              name = RTTRTraits<T>::get_name();
    sequence<text_view> splitted;
    auto                count = name.split(u8"::", splitted);

    // last part is name
    _data->name = splitted.access_at(splitted.size() - 1);

    // fill namespace
    if (count > 1)
    {
        _data->name_space.reserve(count - 1);
        for (auto i = 0; i < count - 1; ++i)
        {
            _data->name_space.push_back(splitted.access_at(i));
        }
    }

    // fill type id
    _data->type_id = RTTRTraits<T>::get_guid();

    // fill size & alignment
    _data->size      = sizeof(T);
    _data->alignment = alignof(T);

    // fill underlying type id
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
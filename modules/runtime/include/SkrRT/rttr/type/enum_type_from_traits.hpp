#pragma once
#include "SkrRT/rttr/type/enum_type.hpp"
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct EnumTypeFromTraits : public EnumType {
    EnumTypeFromTraits()
        : EnumType(RTTRTraits<std::underlying_type_t<T>>::get_type(), RTTRTraits<T>::get_guid(), RTTRTraits<T>::get_name())
    {
    }

    EnumValue value_from_string(string_view str) const override
    {
        T result;
        if (EnumTraits<T>::from_string(str, result))
        {
            return EnumValue(static_cast<std::underlying_type_t<T>>(result));
        }
        else
        {
            return {};
        }
    }
    string value_to_string(const EnumValue& value) const override
    {
        T result;
        if (value.cast_to(result))
        {
            return EnumTraits<T>::to_string(result);
        }
        else
        {
            return u8"";
        }
    }

    int write_binary(const void* dst, skr_binary_writer_t* writer) const override
    {
        if constexpr (is_complete_serde<skr::binary::WriteTrait<T>>())
        {
            return skr::binary::WriteTrait<T>::Write(writer, *reinterpret_cast<const T*>(dst));
        }
        else
        {
            SKR_LOG_ERROR(u8"Type {} is not binary writable", name().c_str());
            return 0;
        }
    }
    int read_binary(void* dst, skr_binary_reader_t* reader) const override
    {
        if constexpr (is_complete_serde<skr::binary::ReadTrait<T>>())
        {
            return skr::binary::ReadTrait<T>::Read(reader, *reinterpret_cast<T*>(dst));
        }
        else
        {
            SKR_LOG_ERROR(u8"Type {} is not binary readable", name().c_str());
            return 0;
        }
    }
    void write_json(const void* dst, skr_json_writer_t* writer) const override
    {
        if constexpr (is_complete_serde<skr::json::WriteTrait<T>>())
        {
            skr::json::WriteTrait<T>::Write(writer, *reinterpret_cast<const T*>(dst));
        }
        else
        {
            SKR_LOG_ERROR(u8"Type {} is not json writable", name().c_str());
        }
    }
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override
    {
        if constexpr (is_complete_serde<skr::json::ReadTrait<T>>())
        {
            return skr::json::ReadTrait<T>::Read(std::move(reader), *reinterpret_cast<T*>(dst));
        }
        else
        {
            SKR_LOG_ERROR(u8"Type {} is not json readable", name().c_str());
            return skr::json::error_code::SUCCESS;
        }
    }
};
} // namespace skr::rttr
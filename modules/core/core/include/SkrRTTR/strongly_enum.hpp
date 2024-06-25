#pragma once
#include <type_traits>
#include "SkrBase/config.h"

namespace skr
{
template <class T>
struct StronglyEnum {
    static_assert(std::is_enum_v<T>, "T must be an enum type");
    using UnderlyingType = std::underlying_type_t<T>;

    inline StronglyEnum() SKR_NOEXCEPT = default;
    inline StronglyEnum(T value) SKR_NOEXCEPT
        : _value(value)
    {
    }
    inline StronglyEnum<T>& operator=(T value) SKR_NOEXCEPT
    {
        this->_value = value;
        return *this;
    }
    inline operator T() const SKR_NOEXCEPT
    {
        return static_cast<T>(_value);
    }

    // getter
    inline T& value() SKR_NOEXCEPT
    {
        return static_cast<T&>(_value);
    }
    inline T value() const SKR_NOEXCEPT
    {
        return static_cast<const T&>(_value);
    }
    inline UnderlyingType& underlying_value() SKR_NOEXCEPT
    {
        return _value;
    }
    inline UnderlyingType underlying_value() const SKR_NOEXCEPT
    {
        return _value;
    }

    // setter
    inline void set_value(T value) SKR_NOEXCEPT
    {
        this->_value = static_cast<T>(value);
    }
    inline void set_underlying_value(UnderlyingType value) SKR_NOEXCEPT
    {
        this->_value = value;
    }

private:
    UnderlyingType _value;
};
} // namespace skr

// binary serde
#include "SkrBase/types.h"
namespace skr::binary
{
template <class T>
struct ReadTrait<StronglyEnum<T>> {
    static bool Read(SBinaryReader* reader, StronglyEnum<T>& value)
    {
        return skr::binary::Archive(reader, value.underlying_value());
    }
};

template <class T>
struct WriteTrait<StronglyEnum<T>> {
    static bool Write(SBinaryWriter* writer, const StronglyEnum<T>& value)
    {
        return skr::binary::Archive(writer, value.underlying_value());
    }
};
} // namespace skr::binary

// json serde
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
template <class T>
struct WriteTrait<StronglyEnum<T>> {
    static bool Write(skr::archive::JsonWriter* writer, const StronglyEnum<T>& value)
    {
        return skr::json::WriteTrait<typename StronglyEnum<T>::UnderlyingType>::Write(writer, value.underlying_value());
    }
};
template <class T>
struct ReadTrait<StronglyEnum<T>> {
    static bool Read(skr::archive::JsonReader* json, StronglyEnum<T>& value)
    {
        return skr::json::ReadTrait<typename StronglyEnum<T>::UnderlyingType>::Read(json, value.underlying_value());
    }
};
} // namespace skr::json
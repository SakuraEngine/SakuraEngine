#pragma once
#include <type_traits>
#include "SkrRT/platform/configure.h"

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

// binary reader & writer
#include "SkrRT/serde/binary/reader_fwd.h"
#include "SkrRT/serde/binary/writer_fwd.h"

namespace skr::binary
{
template <class T>
struct ReadTrait<StronglyEnum<T>> {
    static int Read(skr_binary_reader_t* reader, StronglyEnum<T>& value)
    {
        return skr::binary::Archive(reader, value.underlying_value());
    }
};

template <class T>
struct WriteTrait<const StronglyEnum<T>&> {
    static int Write(skr_binary_writer_t* writer, const StronglyEnum<T>& value)
    {
        return skr::binary::Archive(writer, value.underlying_value());
    }
};
} // namespace skr::binary
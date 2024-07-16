#pragma once
#include <SkrContainersDef/string.hpp>
#include <SkrContainersDef/span.hpp>
#include "SkrBase/meta.h"

// enum traits
namespace skr::rttr
{
template <class T>
struct EnumItem {
    StringView name  = {};
    T          value = {};
};

template <class T>
struct EnumTraits {
    static span<EnumItem<T>> items()
    {
        unimplemented_no_meta(T, "EnumTraits<T>::items is not implemented");
        return {};
    }
    static StringView to_string(const T& value)
    {
        unimplemented_no_meta(T, "EnumTraits<T>::to_string is not implemented");
        return {};
    }
    static bool from_string(StringView str, T& value)
    {
        unimplemented_no_meta(T, "EnumTraits<T>::from_string is not implemented");
        return false;
    }
};
} // namespace skr::rttr

// enum value, used to hold RTTR enum value data
namespace skr::rttr
{
enum class EEnumUnderlyingType
{
    INVALID,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
};
template <typename T>
inline constexpr EEnumUnderlyingType get_underlying_type()
{
    using UnderlyingType = std::underlying_type_t<T>;
    if constexpr (std::is_same_v<UnderlyingType, uint8_t>)
    {
        return EEnumUnderlyingType::UINT8;
    }
    else if constexpr (std::is_same_v<UnderlyingType, uint16_t>)
    {
        return EEnumUnderlyingType::UINT16;
    }
    else if constexpr (std::is_same_v<UnderlyingType, uint32_t>)
    {
        return EEnumUnderlyingType::UINT32;
    }
    else if constexpr (std::is_same_v<UnderlyingType, uint64_t>)
    {
        return EEnumUnderlyingType::UINT64;
    }
    else if constexpr (std::is_same_v<UnderlyingType, int8_t>)
    {
        return EEnumUnderlyingType::INT8;
    }
    else if constexpr (std::is_same_v<UnderlyingType, int16_t>)
    {
        return EEnumUnderlyingType::INT16;
    }
    else if constexpr (std::is_same_v<UnderlyingType, int32_t>)
    {
        return EEnumUnderlyingType::INT32;
    }
    else if constexpr (std::is_same_v<UnderlyingType, int64_t>)
    {
        return EEnumUnderlyingType::INT64;
    }
    else
    {
        return EEnumUnderlyingType::INVALID;
    }
}

struct EnumValue {
    EnumValue()
        : _underlying_type(EEnumUnderlyingType::INVALID)
    {
    }
    EnumValue(uint8_t value)
        : _underlying_type(EEnumUnderlyingType::UINT8)
        , _value_uint8(value)
    {
    }
    EnumValue(int8_t value)
        : _underlying_type(EEnumUnderlyingType::INT8)
        , _value_int8(value)
    {
    }
    EnumValue(uint16_t value)
        : _underlying_type(EEnumUnderlyingType::UINT16)
        , _value_uint16(value)
    {
    }
    EnumValue(int16_t value)
        : _underlying_type(EEnumUnderlyingType::INT16)
        , _value_int16(value)
    {
    }
    EnumValue(uint32_t value)
        : _underlying_type(EEnumUnderlyingType::UINT32)
        , _value_uint32(value)
    {
    }
    EnumValue(int32_t value)
        : _underlying_type(EEnumUnderlyingType::INT32)
        , _value_int32(value)
    {
    }
    EnumValue(uint64_t value)
        : _underlying_type(EEnumUnderlyingType::UINT64)
        , _value_uint64(value)
    {
    }
    EnumValue(int64_t value)
        : _underlying_type(EEnumUnderlyingType::INT64)
        , _value_int64(value)
    {
    }

    // getter
    SKR_INLINE EEnumUnderlyingType underlying_type() const { return _underlying_type; }
    SKR_INLINE uint8_t             value_uint8() const { return _value_uint8; }
    SKR_INLINE int8_t              value_int8() const { return _value_int8; }
    SKR_INLINE uint16_t            value_uint16() const { return _value_uint16; }
    SKR_INLINE int16_t             value_int16() const { return _value_int16; }
    SKR_INLINE uint32_t            value_uint32() const { return _value_uint32; }
    SKR_INLINE int32_t             value_int32() const { return _value_int32; }
    SKR_INLINE uint64_t            value_uint64() const { return _value_uint64; }
    SKR_INLINE int64_t             value_int64() const { return _value_int64; }

    // caster
    template <typename T>
    SKR_INLINE bool cast_to(T& result) const
    {
        if (!is_valid())
        {
            return false;
        }

        EEnumUnderlyingType underlying_type = get_underlying_type<T>();
        if (underlying_type == EEnumUnderlyingType::INVALID)
        {
            return false;
        }
        else if (underlying_type == EEnumUnderlyingType::UINT8)
        {
            if ((_underlying_type >= EEnumUnderlyingType::UINT8 && _underlying_type <= EEnumUnderlyingType::UINT64) ||
                (_underlying_type >= EEnumUnderlyingType::INT16 && _underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(_value_uint8);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::INT8)
        {
            if ((_underlying_type >= EEnumUnderlyingType::INT8 && _underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(_value_int8);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::UINT16)
        {
            if ((_underlying_type >= EEnumUnderlyingType::UINT16 && _underlying_type <= EEnumUnderlyingType::UINT64) ||
                (_underlying_type >= EEnumUnderlyingType::INT32 && _underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(_value_uint16);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::INT16)
        {
            if (_underlying_type >= EEnumUnderlyingType::INT16 && _underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(_value_int16);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::UINT32)
        {
            if ((_underlying_type >= EEnumUnderlyingType::UINT32 && _underlying_type <= EEnumUnderlyingType::UINT64) ||
                _underlying_type == EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(_value_uint32);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::INT32)
        {
            if (_underlying_type >= EEnumUnderlyingType::INT32 && _underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(_value_int32);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::UINT64)
        {
            if (_underlying_type == EEnumUnderlyingType::UINT64)
            {
                result = static_cast<T>(_value_uint64);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (underlying_type == EEnumUnderlyingType::INT64)
        {
            if (_underlying_type >= EEnumUnderlyingType::INT64 && _underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(_value_int64);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            SKR_UNREACHABLE_CODE();
            return false;
        }
    }

    // validate
    SKR_INLINE bool is_valid() const { return _underlying_type != EEnumUnderlyingType::INVALID; }
    SKR_INLINE      operator bool() const { return is_valid(); }

private:
    EEnumUnderlyingType _underlying_type;
    union
    {
        uint8_t  _value_uint8;
        int8_t   _value_int8;
        uint16_t _value_uint16;
        int16_t  _value_int16;
        uint32_t _value_uint32;
        int32_t  _value_int32;
        uint64_t _value_uint64;
        int64_t  _value_int64;
    };
};

} // namespace skr::rttr

// strongly enum, used to hold pure c style enum (without underlying type)
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

// strongly enum binary serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <class T>
struct BinSerde<StronglyEnum<T>> {
    inline static bool read(SBinaryReader* r, StronglyEnum<T>& v)
    {
        return bin_read(r, v.underlying_value());
    }
    inline static bool write(SBinaryWriter* w, const StronglyEnum<T>& v)
    {
        return bin_write(w, v.underlying_value());
    }
};
} // namespace skr

// strongly enum json serde
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
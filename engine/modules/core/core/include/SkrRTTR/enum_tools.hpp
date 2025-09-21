#pragma once
#include <SkrContainersDef/string.hpp>
#include <SkrContainersDef/span.hpp>

// enum value, used to hold RTTR enum value data
namespace skr
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
    if constexpr (std::is_enum_v<T>)
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
    else
    {
        using UnderlyingType = T;
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
}

struct EnumValue
{
    EnumValue()
        : _underlying_type(EEnumUnderlyingType::INVALID)
    {
    }
    EnumValue(uint8_t value)
        : _underlying_type(EEnumUnderlyingType::UINT8)
        , _value_unsigned(value)
    {
    }
    EnumValue(int8_t value)
        : _underlying_type(EEnumUnderlyingType::INT8)
        , _value_signed(value)
    {
    }
    EnumValue(uint16_t value)
        : _underlying_type(EEnumUnderlyingType::UINT16)
        , _value_unsigned(value)
    {
    }
    EnumValue(int16_t value)
        : _underlying_type(EEnumUnderlyingType::INT16)
        , _value_signed(value)
    {
    }
    EnumValue(uint32_t value)
        : _underlying_type(EEnumUnderlyingType::UINT32)
        , _value_unsigned(value)
    {
    }
    EnumValue(int32_t value)
        : _underlying_type(EEnumUnderlyingType::INT32)
        , _value_signed(value)
    {
    }
    EnumValue(uint64_t value)
        : _underlying_type(EEnumUnderlyingType::UINT64)
        , _value_unsigned(value)
    {
    }
    EnumValue(int64_t value)
        : _underlying_type(EEnumUnderlyingType::INT64)
        , _value_signed(value)
    {
    }

    // getter
    SKR_INLINE EEnumUnderlyingType underlying_type() const { return _underlying_type; }
    SKR_INLINE uint8_t value_uint8() const { return static_cast<uint8_t>(_value_unsigned); }
    SKR_INLINE int8_t value_int8() const { return static_cast<int8_t>(_value_signed); }
    SKR_INLINE uint16_t value_uint16() const { return static_cast<uint16_t>(_value_unsigned); }
    SKR_INLINE int16_t value_int16() const { return static_cast<int16_t>(_value_signed); }
    SKR_INLINE uint32_t value_uint32() const { return static_cast<uint32_t>(_value_unsigned); }
    SKR_INLINE int32_t value_int32() const { return static_cast<int32_t>(_value_signed); }
    SKR_INLINE uint64_t value_uint64() const { return static_cast<uint64_t>(_value_unsigned); }
    SKR_INLINE int64_t value_int64() const { return static_cast<int64_t>(_value_signed); }

    // sign
    SKR_INLINE bool is_unsigned() const
    {
        return _underlying_type >= EEnumUnderlyingType::UINT8 && _underlying_type <= EEnumUnderlyingType::UINT64;
    }
    SKR_INLINE bool is_signed() const
    {
        return _underlying_type >= EEnumUnderlyingType::INT8 && _underlying_type <= EEnumUnderlyingType::INT64;
    }
    SKR_INLINE int64_t value_signed() const
    {
        SKR_ASSERT(is_signed());
        return _value_signed;
    }
    SKR_INLINE uint64_t value_unsigned() const
    {
        SKR_ASSERT(is_unsigned());
        return _value_unsigned;
    }

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

        switch (_underlying_type)
        {
        case EEnumUnderlyingType::UINT8:
            if ((underlying_type >= EEnumUnderlyingType::UINT8 && underlying_type <= EEnumUnderlyingType::UINT64) ||
                (underlying_type >= EEnumUnderlyingType::INT16 && underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(value_uint8());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::INT8:
            if ((underlying_type >= EEnumUnderlyingType::INT8 && underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(value_int8());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::UINT16:
            if ((underlying_type >= EEnumUnderlyingType::UINT16 && underlying_type <= EEnumUnderlyingType::UINT64) ||
                (underlying_type >= EEnumUnderlyingType::INT32 && underlying_type <= EEnumUnderlyingType::INT64))
            {
                result = static_cast<T>(value_uint16());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::INT16:
            if (underlying_type >= EEnumUnderlyingType::INT16 && underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(value_int16());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::UINT32:
            if ((underlying_type >= EEnumUnderlyingType::UINT32 && underlying_type <= EEnumUnderlyingType::UINT64) ||
                underlying_type == EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(value_uint32());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::INT32:
            if (underlying_type >= EEnumUnderlyingType::INT32 && underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(value_int32());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::UINT64:
            if (underlying_type == EEnumUnderlyingType::UINT64)
            {
                result = static_cast<T>(value_uint64());
                return true;
            }
            else
            {
                return false;
            }
        case EEnumUnderlyingType::INT64:
            if (underlying_type >= EEnumUnderlyingType::INT64 && underlying_type <= EEnumUnderlyingType::INT64)
            {
                result = static_cast<T>(value_int64());
                return true;
            }
            else
            {
                return false;
            }
        default:
            SKR_UNREACHABLE_CODE();
            return false;
        }
    }

    // validate
    SKR_INLINE bool is_valid() const { return _underlying_type != EEnumUnderlyingType::INVALID; }
    SKR_INLINE operator bool() const { return is_valid(); }

    // compare
    SKR_INLINE bool operator==(const EnumValue& rhs) const
    {
        if (_underlying_type != rhs._underlying_type)
        {
            return false;
        }

        if (is_signed())
        {
            return _value_signed == rhs._value_signed;
        }
        else
        {
            return _value_unsigned == rhs._value_unsigned;
        }
    }

    // hash
    SKR_INLINE skr_hash _skr_hash() const
    {
        auto seed = Hash<EEnumUnderlyingType>()(_underlying_type);
        if (is_signed())
        {
            return hash_combine(
                seed,
                Hash<int64_t>()(_value_signed)
            );
        }
        else
        {
            return hash_combine(
                seed,
                Hash<uint64_t>()(_value_unsigned)
            );
        }
    }

private:
    EEnumUnderlyingType _underlying_type;
    union
    {
        int64_t _value_signed;
        uint64_t _value_unsigned;
    };
};

} // namespace skr

// strongly enum, used to hold pure c style enum (without underlying type)
namespace skr
{
template <class T>
struct EnumAsValue
{
    static_assert(std::is_enum_v<T>, "T must be an enum type");
    using UnderlyingType = std::underlying_type_t<T>;

    inline EnumAsValue() SKR_NOEXCEPT = default;
    inline EnumAsValue(T value) SKR_NOEXCEPT
        : _value(value)
    {
    }
    inline EnumAsValue<T>& operator=(T value) SKR_NOEXCEPT
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

// enum as value serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename T>
struct Serialize<EnumAsValue<T>>
{
    using UT = typename EnumAsValue<T>::UnderlyingType;

    inline static void read(ArchiveRead& r, EnumAsValue<T>& v)
    {
        r.value<UT>(v.underlying_value());
    }
    inline static void write(ArchiveWrite& w, const EnumAsValue<T>& v)
    {
        w.value<UT>(v.underlying_value());
    }
};
} // namespace skr
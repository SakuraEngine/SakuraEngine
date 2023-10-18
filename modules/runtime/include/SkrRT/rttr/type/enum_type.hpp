#pragma once
#include "SkrRT/containers/string.hpp"
#include "SkrRT/rttr/type/type.hpp"

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
            SKR_UNREACHABLE_CODE()
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

struct SKR_RUNTIME_API EnumType : public Type {
    EnumType(Type* underlying_type, GUID type_id, string name);

    SKR_INLINE Type* underlying_type() const { return _underlying_type; }

    void   call_ctor(void* ptr) const override;
    void   call_dtor(void* ptr) const override;
    void   call_copy(void* dst, const void* src) const override;
    void   call_move(void* dst, void* src) const override;
    void   call_assign(void* dst, const void* src) const override;
    void   call_move_assign(void* dst, void* src) const override;
    size_t call_hash(const void* ptr) const override;

    virtual EnumValue value_from_string(string_view str) const      = 0;
    virtual string    value_to_string(const EnumValue& value) const = 0;

private:
    Type* _underlying_type;
};
} // namespace skr::rttr
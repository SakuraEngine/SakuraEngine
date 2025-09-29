#pragma once
#include "CppSL/Expected.hpp"
#include "double-conversion/ieee.h"

namespace skr::CppSL {

struct IntValue
{
public:
    IntValue(int8_t v);
    IntValue(uint8_t v);
    IntValue(int16_t v);
    IntValue(uint16_t v);
    IntValue(int32_t v);
    IntValue(uint32_t v);
    IntValue(int64_t v);
    IntValue(uint64_t v);

    template <typename I> requires std::is_integral_v<I>
    ssl::Expected<I> value() const
    {
        const auto required_bitwidth = sizeof(I) * 8;
        const auto required_signed = std::is_signed_v<I>;

        const bool bitwidth_capable = 
            (_signed && required_signed) ? _bitwidth <= required_bitwidth :
            (_signed && !required_signed) ? _bitwidth <= required_bitwidth + 1 :

            (!_signed && !required_signed) ? _bitwidth <= required_bitwidth :
            (!_signed && required_signed) ? false : false;

        if (!bitwidth_capable)
        {
            return ssl::createStringError("value is too large for the type");
        }

        if (_signed)
        {
            auto v = *reinterpret_cast<const int64_t*>(&_storage);
            return static_cast<I>(v);
        }
        else
        {
            auto v = *reinterpret_cast<const uint64_t*>(&_storage);
            return static_cast<I>(v);
        }
    }

    const bool is_signed() const { return _signed; }
    const uint32_t bitwidth() const { return _bitwidth; }

private:
    std::aligned_storage<64, alignof(int64_t)> _storage;
    uint32_t _bitwidth = 32;
    bool _signed = false;
};

struct FloatValue
{
public:
    FloatValue(const FloatValue& v);
    FloatValue(float v);
    FloatValue(double v);
    // support decimal & hexfloat: -123.45 or 0x10.1p0
    FloatValue(std::string_view v);

    const double_conversion::Double ieee;

private:
    uint32_t _bitwidth = 32;
    int32_t _processed_chars_count = 0;
};

} // namespace skr::CppSL
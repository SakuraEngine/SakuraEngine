#include "CppSL/Constant.hpp"
#include "double-conversion/string-to-double.h"

namespace skr::CppSL
{

const double_conversion::StringToDoubleConverter gDoubleConverter = double_conversion::StringToDoubleConverter(
    double_conversion::StringToDoubleConverter::ALLOW_HEX_FLOATS |
        double_conversion::StringToDoubleConverter::ALLOW_TRAILING_JUNK |
        double_conversion::StringToDoubleConverter::ALLOW_LEADING_SPACES,
    0.0,
    0.0,
    "Infinity",
    "NaN");

IntValue::IntValue(int8_t v)
    : _bitwidth(8)
    , _signed(true)
{
    new (&_storage) int64_t(v);
}
IntValue::IntValue(int16_t v)
    : _bitwidth(16)
    , _signed(true)
{
    new (&_storage) int64_t(v);
}
IntValue::IntValue(int32_t v)
    : _bitwidth(32)
    , _signed(true)
{
    new (&_storage) int64_t(v);
}
IntValue::IntValue(int64_t v)
    : _bitwidth(64)
    , _signed(true)
{
    new (&_storage) int64_t(v);
}

IntValue::IntValue(uint8_t v)
    : _bitwidth(8)
    , _signed(false)
{
    new (&_storage) uint64_t(v);
}
IntValue::IntValue(uint16_t v)
    : _bitwidth(16)
    , _signed(false)
{
    new (&_storage) uint64_t(v);
}
IntValue::IntValue(uint32_t v)
    : _bitwidth(32)
    , _signed(false)
{
    new (&_storage) uint64_t(v);
}
IntValue::IntValue(uint64_t v)
    : _bitwidth(64)
    , _signed(false)
{
    new (&_storage) uint64_t(v);
}

FloatValue::FloatValue(const FloatValue& v)
{
    std::memcpy((void*)this, &v, sizeof(FloatValue));
}

FloatValue::FloatValue(float v)
    : _bitwidth(32)
    , ieee(v)
{
}

FloatValue::FloatValue(double v)
    : _bitwidth(64)
    , ieee(v)
{
}

FloatValue::FloatValue(std::string_view v)
    : _bitwidth(32)
    , ieee(gDoubleConverter.StringToDouble(v.data(), v.size(), &_processed_chars_count))
{
}

} // namespace skr::CppSL
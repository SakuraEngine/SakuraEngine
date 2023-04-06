#include "format.h"
#include "text.h"

OPEN_STRING_NS_BEGIN

namespace details
{
    [[nodiscard]] constexpr i32 get_integer_digit_count(const i64 value, const i32 base)
    {
        if(value == 0)
            return 1;
        i64 remaining = value;
        i32 count = 0;
        while(remaining != 0)
        {
            remaining /= base;
            ++count;
        }
        return count;
    }

    [[nodiscard]] constexpr ochar8_t from_digit(const i64 digit)
    {
        constexpr ochar8_t digits[] = OSTR_UTF8("0123456789abcdef");
        return digits[digit];
    }
}

codeunit_sequence details::format_integer(const i64& value, const codeunit_sequence_view& specification)
{
    ochar8_t type = 'd';
    i32 holding = -1;
    bool with_prefix = false;
    if (!specification.is_empty())
    {
        codeunit_sequence_view parsing = specification;
        if (OSTR_UTF8("bcdox"_cuqv).contains(parsing[-1]))
        {
            type = parsing[-1];
            parsing = parsing.subview({ '[', 0, -2, ']' });
        }
        if(!parsing.is_empty())
        {
            const i32 zero_index = parsing.index_of('0');
            if(zero_index != index_invalid)
            {
                const codeunit_sequence_view holding_view = parsing.subview({ '(', zero_index, '~' });
                const auto [ last, error ] = std::from_chars(holding_view.c_str(), (const char*)holding_view.last(), holding);
                /*
                if(last != holding_view.last())
                    throw format_error("Invalid format specification [{}]!"_cuqv, specification);
                */
                parsing = parsing.subview({ '[', 0, zero_index, ')' });
            }
        }
        if(!parsing.is_empty())
        {
            if(parsing[-1] == '#')
            {
                with_prefix = true;
                parsing = parsing.subview({ '[', 0, -1, ')' });
            }
        }
        /*
        if(!parsing.is_empty())
            throw format_error("Invalid format specification [{}]!"_cuqv, specification);
        */
    }
    i32 base = 10;
    codeunit_sequence_view prefix;
    switch (type)
    {
    case 'b':
        base = 2;
        prefix = OSTR_UTF8("0b"_cuqv);
        break;
    case 'c':
        return codeunit_sequence{ static_cast<ochar8_t>(value) };
    case 'd':
        break;
    case 'o':
        base = 8;
        prefix = OSTR_UTF8("0o"_cuqv);
        break;
    case 'x':
        base = 16;
        prefix = OSTR_UTF8("0x"_cuqv);
        break;
    default:
        break;
    }
    if(!with_prefix)
        prefix = OSTR_UTF8(""_cuqv);
    
    const codeunit_sequence_view sign = (value < 0) ? OSTR_UTF8("-"_cuqv) : OSTR_UTF8(""_cuqv);
    const i32 digit_count = details::get_integer_digit_count(value, base);
    const i32 preserve = (holding > digit_count) ? holding : digit_count;
    const i32 zero_count = preserve - digit_count;
    codeunit_sequence result( sign.size() + prefix.size() + preserve );
    result .append(sign) .append(prefix) .append('0', zero_count);
    const i32 digits_start = result.size();
    i64 remaining = value >= 0 ? value : -value;
    for(i32 i = 0; i < digit_count; ++i)
    {
        const ochar8_t digit = details::from_digit(remaining % base); 
        remaining /= base;
        result.append(digit);
    }
    result.reverse({ '[', digits_start, '~' });
    return result;
}

codeunit_sequence details::format_float(const float& value, const codeunit_sequence_view& specification)
{
    if (std::isinf(value))
        return codeunit_sequence{ value < 0 ? OSTR_UTF8("-inf"_cuqv) : OSTR_UTF8("inf"_cuqv) };
    if (std::isnan(value))
        return codeunit_sequence{ OSTR_UTF8("nan"_cuqv) };
    i32 precision = -1;
    ochar8_t type = 'g';
    if (!specification.is_empty())
    {
        codeunit_sequence_view parsing = specification;
        if(OSTR_UTF8("aefg"_cuqv).contains( parsing[-1] ))
        {    
            type = parsing[-1];
            parsing = parsing.subview({ '[', 0, -2, ']' });
        }
        if(!parsing.is_empty())
        {
            const i32 dot_index = parsing.index_of('.');
            if(dot_index != index_invalid)
            {
                const codeunit_sequence_view precision_view = parsing.subview({ '(', dot_index, '~' });
                const auto [ last, error ] = std::from_chars(precision_view.c_str(), (const char*)precision_view.last(), precision);
                /*
                if(last != precision_view.last())
                    throw format_error("Invalid format specification [{}]!"_cuqv, specification);
                */
                parsing = parsing.subview({ '[', 0, dot_index, ')' });
            }
        }
        /*
        if(!parsing.is_empty())
            throw format_error("Invalid format specification [{}]!"_cuqv, specification);
        */
    }
    switch (type) 
    {
    case 'a':
        break;
    case 'e':
        break;
    case 'f':
        break;
    case 'g':
        break;
    default:
        break;
    }
    /*
    static constexpr i32 max_precision = 9;
    if(precision > max_precision)
        throw format_error("Too high precision for float [{}]!"_cuqv, precision);
    */
    codeunit_sequence result;
    const bool negative = value < 0;
    if(negative)
        result.append(OSTR_UTF8("-"));
    float remaining = negative ? -value : value;
    const i32 decimal = static_cast<i32>(remaining);
    result.append(format_integer(decimal, { }));
    remaining -= decimal;
    static constexpr float epsilon = 1e-3f;
    if(precision == -1)
    {
        i32 floating = 1;
        while(true)
        {
            remaining *= 10;
            floating *= 10;
            const i32 ones = static_cast<i32>(remaining);
            floating += ones;
            remaining -= ones;
            if(remaining < epsilon && remaining > -epsilon)
                break;
        }
        if(floating > 10)
        {
            const i32 dot_position = result.size();
            result
            .append(format_integer(floating, { }))
            .write_at(dot_position, '.');
        }
    }
    else 
    {
        i32 floating = 1;
        for(i32 i = 0; i < precision; ++i)
        {
            remaining *= 10;
            floating *= 10;
            const i32 ones = static_cast<i32>(remaining);
            floating += ones;
            remaining -= ones;
        }
        const i32 dot_position = result.size();
        result
        .append(format_integer(floating, { }))
        .write_at(dot_position, '.');
    }
    return result;
}

OPEN_STRING_NS_END

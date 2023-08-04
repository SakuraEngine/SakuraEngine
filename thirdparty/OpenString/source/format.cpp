
#include "common/definitions.h"
#include "format.h"
#include "text.h"

namespace ostr
{
    namespace details
    {
        [[nodiscard]] constexpr u64 get_integer_digit_count(const i64 value, const i64 base)
        {
            if(value == 0)
                return 1;
            i64 remaining = value;
            u64 count = 0;
            while(remaining != 0)
            {
                remaining /= base;
                ++count;
            }
            return count;
        }
        
        [[nodiscard]] constexpr u64 get_integer_digit_count(const u64 value, const u64 base)
        {
            if(value == 0)
                return 1;
            u64 remaining = value;
            u64 count = 0;
            while(remaining != 0)
            {
                remaining /= base;
                ++count;
            }
            return count;
        }

        [[nodiscard]] constexpr ochar8_t from_digit(const u64 digit)
        {
            constexpr ochar8_t digits[] = u8"0123456789abcdef";
            return digits[digit];
        }
    
        codeunit_sequence format_integer(const u64& value, const codeunit_sequence_view& specification)
        {
            ochar8_t type = u8'd';
            ochar8_t holder = u8'0';
            u64 holding = global_constant::SIZE_INVALID;
            bool with_prefix = false;
            if (!specification.is_empty())
            {
                codeunit_sequence_view parsing = specification;
                if (u8"bcdox"_cuqv.contains(parsing.read_from_last(0)))
                {
                    type = parsing.read_from_last(0);
                    parsing = parsing.subview(0, parsing.size() - 1);
                }
                if(!parsing.is_empty())
                {
                    if(const u64 holder_index = parsing.index_of_any(u8"0 "_cuqv); holder_index != global_constant::INDEX_INVALID)
                    {
                        holder = parsing.read_at(holder_index);
                        const codeunit_sequence_view holding_view = parsing.subview(holder_index + 1);
                        const auto [ last, error ] = std::from_chars((const char*)holding_view.data(), 
                            (const char*)holding_view.cend().data(), holding);
                        OPEN_STRING_CHECK(last == (const char*)holding_view.cend().data(), u8"Invalid format specification [{}]!", specification);
                        parsing = parsing.subview(0, holder_index);
                    }
                }
                if(!parsing.is_empty())
                {
                    if(parsing.read_from_last(0) == '#')
                    {
                        with_prefix = true;
                        parsing = parsing.subview(0, parsing.size() - 1);
                    }
                }
                OPEN_STRING_CHECK(parsing.is_empty(), u8"Invalid format specification [{}]!", specification);
            }
            u64 base = 10;
            codeunit_sequence_view prefix;
            switch (type)
            {
            case 'b':
                {
                    base = 2;
                    prefix = u8"0b"_cuqv;
                }
                break;
            case 'c':
                {
                    codeunit_sequence result;
                    result.append(static_cast<ochar8_t>(value));
                    return result;
                }
            case 'd':
                break;
            case 'o':
                {
                    base = 8;
                    prefix = u8"0o"_cuqv;
                }
                break;
            case 'x':
                {
                    base = 16;
                    prefix = u8"0x"_cuqv;
                }
                break;
            default:
                break;
            }
            if(!with_prefix)
                prefix = u8""_cuqv;
        
            const u64 digit_count = get_integer_digit_count(value, base);
            const u64 preserve = holding == global_constant::SIZE_INVALID ? digit_count : maximum(holding, digit_count);
            const u64 holder_count = preserve - digit_count;
            codeunit_sequence result( prefix.size() + preserve );
            result
            .append(prefix)
            .append(holder, holder_count);
            u64 remaining = value;
            for(u64 i = 0; i < digit_count; ++i)
            {
                const ochar8_t digit = from_digit(remaining % base); 
                remaining /= base;
                result.append(digit);
            }
            result.reverse();
            return result;
        }

        codeunit_sequence format_integer(const i64& value, const codeunit_sequence_view& specification)
        {
            ochar8_t type = 'd';
            u64 holding = global_constant::SIZE_INVALID;
            bool with_prefix = false;
            if (!specification.is_empty())
            {
                codeunit_sequence_view parsing = specification;
                if (u8"bcdox"_cuqv.contains(parsing.read_from_last(0)))
                {
                    type = parsing.read_from_last(0);
                    parsing = parsing.subview(0, parsing.size() - 1);
                }
                if(!parsing.is_empty())
                {
                    if(const u64 zero_index = parsing.index_of('0'); zero_index != global_constant::INDEX_INVALID)
                    {
                        const codeunit_sequence_view holding_view = parsing.subview(zero_index + 1);
                        const auto [ last, error ] = std::from_chars((const char*)holding_view.data(), (const char*)holding_view.cend().data(), holding);
                        OPEN_STRING_CHECK(last == (const char*)holding_view.cend().data(), u8"Invalid format specification [{}]!", specification);
                        parsing = parsing.subview(0, zero_index);
                    }
                }
                if(!parsing.is_empty())
                {
                    if(parsing.read_from_last(0) == '#')
                    {
                        with_prefix = true;
                        parsing = parsing.subview(0, parsing.size() - 1);
                    }
                }
                OPEN_STRING_CHECK(parsing.is_empty(), u8"Invalid format specification [{}]!", specification);
            }
            i64 base = 10;
            codeunit_sequence_view prefix;
            switch (type)
            {
            case 'b':
                {
                    base = 2;
                    prefix = u8"0b"_cuqv;
                }
                break;
            case 'c':
                {
                    codeunit_sequence result;
                    result.append(static_cast<ochar8_t>(value));
                    return result;
                }
            case 'd':
                break;
            case 'o':
                {
                    base = 8;
                    prefix = u8"0o"_cuqv;
                }
                break;
            case 'x':
                {
                    base = 16;
                    prefix = u8"0x"_cuqv;
                }
                break;
            default:
                break;
            }
            if(!with_prefix)
                prefix = u8""_cuqv;
        
            const codeunit_sequence_view sign = (value < 0) ? u8"-"_cuqv : u8""_cuqv;
            const u64 digit_count = get_integer_digit_count(value, base);
            const u64 preserve = holding == global_constant::SIZE_INVALID ? digit_count : maximum(holding, digit_count);
            const u64 zero_count = preserve - digit_count;
            codeunit_sequence result( sign.size() + prefix.size() + preserve );
            result
            .append(sign)
            .append(prefix)
            .append('0', zero_count);
            const u64 digits_start = result.size();
            i64 remaining = value >= 0 ? value : -value;
            for(u64 i = 0; i < digit_count; ++i)
            {
                const ochar8_t digit = from_digit(remaining % base); 
                remaining /= base;
                result.append(digit);
            }
            result.reverse(digits_start);
            return result;
        }

        codeunit_sequence format_float(const f64& value, const codeunit_sequence_view& specification)
        {
            if (std::isinf(value))
                return codeunit_sequence{ value < 0 ? u8"-inf"_cuqv : u8"inf"_cuqv };
            if (std::isnan(value))
                return codeunit_sequence{ u8"nan"_cuqv };
            u64 precision = global_constant::SIZE_INVALID;
            // ochar8_t type = 'g';
            if (!specification.is_empty())
            {
                codeunit_sequence_view parsing = specification;
                if(u8"aefg"_cuqv.contains( parsing.read_from_last(0) ))
                {    
                    // type = parsing.read_from_last(0);
                    parsing = parsing.subview(0, parsing.size() - 1);
                }
                if(!parsing.is_empty())
                {
                    if(const u64 dot_index = parsing.index_of('.'); dot_index != global_constant::INDEX_INVALID)
                    {
                        const codeunit_sequence_view precision_view = parsing.subview(dot_index + 1);
                        const auto [ last, error ] = std::from_chars((const char*)precision_view.data(), 
                            (const char*)precision_view.cend().data(), precision);
                        OPEN_STRING_CHECK(last == (const char*)precision_view.cend().data(), u8"Invalid format specification [{}]!", specification);
                        parsing = parsing.subview(0, dot_index);
                    }
                }
                OPEN_STRING_CHECK(parsing.is_empty(), u8"Invalid format specification [{}]!", specification);
            }
            // switch (type) 
            // {
            // case 'a':
            //     break;
            // case 'e':
            //     break;
            // case 'f':
            //     break;
            // case 'g':
            //     break;
            // default:
            //     break;
            // }
            static constexpr u64 max_precision = 9;
            OPEN_STRING_CHECK(precision == global_constant::SIZE_INVALID || precision <= max_precision, u8"Too high precision for float type [{}]!", precision);
            codeunit_sequence result;
            const bool negative = value < 0;
            if(negative)
                result.append(u8"-");
            f64 remaining = negative ? -value : value;
            const u64 decimal = static_cast<u64>(remaining);
            remaining -= static_cast<f64>(decimal);
            if(precision == global_constant::SIZE_INVALID)
            {
                static constexpr u64 max_floating = power(10, max_precision + 1);
                u64 floating = 1;
                u64 count_nine = 0;
                while(floating <= max_floating)
                {
                    remaining *= 10;
                    floating *= 10;
                    const u64 ones = static_cast<u64>(remaining);
                    floating += ones;
                    remaining -= static_cast<f64>(ones);
                    if(ones == 9)
                    {
                        ++count_nine;
                        if(count_nine == global_constant::TOLERANCE_EXPONENT)
                        {
                            floating += 1;
                            floating = floating / power(10, global_constant::TOLERANCE_EXPONENT);
                            break;
                        }
                    }
                    else
                    {
                        count_nine = 0;
                    }
                    if(remaining < global_constant::TOLERANCE)
                        break;
                }
                if(floating > 10)
                {
                    const codeunit_sequence decimal_part = format_integer(decimal, { });
                    const u64 dot_position = result.size() + decimal_part.size();
                    const codeunit_sequence floating_part = format_integer(floating, { });
                    result.reserve(dot_position + floating_part.size());
                    result  .append(decimal_part)
                            .append(floating_part)
                            .write_at(dot_position, '.');
                }
                if(floating == 2)
                {
                    result.append(format_integer(decimal + 1, { }));
                }
            }
            else 
            {
                u64 floating = 1;
                for(u64 i = 0; i < precision; ++i)
                {
                    remaining *= 10;
                    floating *= 10;
                    const u64 ones = static_cast<u64>(remaining);
                    floating += ones;
                    remaining -= static_cast<f64>(ones);
                }
                const codeunit_sequence decimal_part = format_integer(decimal, { });
                const u64 dot_position = result.size() + decimal_part.size();
                const codeunit_sequence floating_part = format_integer(floating, { });
                result.reserve(dot_position + floating_part.size());
                result  .append(decimal_part)
                        .append(floating_part)
                        .write_at(dot_position, '.');
            }
            return result;
        }
    }
}

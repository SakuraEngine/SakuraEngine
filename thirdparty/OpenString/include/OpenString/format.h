// OpenString - formatting library
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once

#include <array>
#include <cmath>
#include <stdexcept>
#include <charconv>
#include "codeunit_sequence.h"

OPEN_STRING_NS_BEGIN

class format_error final : public std::runtime_error
{
public:
    explicit format_error(const codeunit_sequence_view& message)
        : std::runtime_error(message.c_str()) 
    { }

    template<class Arg1, class...Args>
    format_error(const codeunit_sequence_view& message_format, Arg1&& arg1, Args&&...args);
};

template<class T>
struct formatter
{
    static codeunit_sequence format_argument(const T& value, const codeunit_sequence_view& specification);
};

namespace details
{
    class format_view
    {
    public:
    // code-region-start: constructors

        explicit constexpr format_view(const codeunit_sequence_view& format) noexcept
            : format_(format)
        { }

    // code-region-end: constructors

    // code-region-start: iterators

        enum class run_type : u8
        {
            plain_text,
            escaped_brace,
            formatter,
            ending
        };

        struct const_iterator
        {
            constexpr const_iterator() noexcept = default;

            explicit constexpr const_iterator(const codeunit_sequence_view& v) noexcept
                : format(v)
            { }

            constexpr const_iterator& initialize() noexcept
            {
                const auto [ first_type, first_range ] = this->parse_run(0);
                this->type = first_type;
                this->range = first_range;
                return *this;
            }

            [[nodiscard]] constexpr std::tuple<run_type, codeunit_sequence_view> operator*() const noexcept
            {
                return { this->type, this->format.subview(this->range) };
            }

            constexpr const_iterator& operator++()
            {
                if(this->range.is_infinity())
                {
                    this->type = run_type::ending;
                    this->range = index_interval::empty();
                    return *this;
                }
                const auto [ next_type, next_range ] = this->parse_run(this->range.get_exclusive_max());
                this->type = next_type;
                this->range = next_range;
                return *this;
            }

            constexpr const_iterator operator++(int) noexcept
            {
                const const_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            [[nodiscard]] constexpr bool operator==(const const_iterator& rhs) const noexcept
            {
                return this->type == rhs.type && this->range == rhs.range;
            }

            [[nodiscard]] constexpr bool operator!=(const const_iterator& rhs) const noexcept
            {
                return !(*this == rhs);
            }

            [[nodiscard]] constexpr std::tuple<run_type, index_interval> parse_run(const i32 from) const
            {
                const i32 index = this->format.index_any_of("{}"_cuqv, { '[', from, '~' });
                if(index == index_invalid)
                    return { run_type::plain_text, { '[', from, '~' } };
                if(from != index)
                    return { run_type::plain_text, { '[', from, index, ')' } };
                if(format[index] == format[index+1])
                    return { run_type::escaped_brace, { '[', index, index + 1, ']' } };
                if(format[index] == '}')
                    throw format_error("Unclosed right brace is not allowed!"_cuqv);
                const i32 index_next = this->format.index_any_of("{}"_cuqv, { '(', index, '~' });
                if(index_next == index_invalid || format[index_next] == '{')
                    throw format_error("Unclosed left brace is not allowed!"_cuqv);
                return { run_type::formatter, { '[', index, index_next, ']' } };
            }
            
            codeunit_sequence_view format;
            run_type type = run_type::ending;
            index_interval range = index_interval::empty();
        };
        
        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return const_iterator{ this->format_ }.initialize();
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return const_iterator{ this->format_ };
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        {
            return this->begin();
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept
        {
            return this->end();
        }

    // code-region-end: iterators
    
    private:
        codeunit_sequence_view format_;
    };

    template <class T>
    codeunit_sequence formatter_intermediate(const void* value, const codeunit_sequence_view specification)
    {
        return formatter<T>::format_argument(*static_cast<const T*>(value), specification);
    }

    struct format_argument_pack
    {
        using formatter_type = codeunit_sequence(*)(const void* value, codeunit_sequence_view specification);

        const void* value;
        formatter_type formatter;

        template<class T>
        explicit constexpr format_argument_pack(const T& v) noexcept
            : value(static_cast<const void*>(&v))
            , formatter(formatter_intermediate<T>)
        { }

        [[nodiscard]] codeunit_sequence format(const codeunit_sequence_view specification) const
        {
            return this->formatter(this->value, specification);
        }
    };

    template<class...Args>
    codeunit_sequence format(const format_view& format, const Args&...args)
    {
        constexpr i32 argument_count = sizeof...(Args);
        const std::array<format_argument_pack, argument_count> arguments {{ format_argument_pack{ args } ... }};

        constexpr i32 manual_index = -1;
        i32 next_index = 0;
        codeunit_sequence result;
        for(const auto [ type, run ] : format)
        {
            switch (type) 
            {
            case format_view::run_type::plain_text:
                result += run;
                break;
            case format_view::run_type::escaped_brace:
                result += run[0];
                break;
            case format_view::run_type::formatter:
            {    
                const auto [ index_run, specification ] = run.subview({ '[', 1, -2, ']' }).split(":"_cuqv);
                i32 current_index = next_index;
                if (index_run.is_empty())
                {
                    if(next_index == manual_index)
                        throw format_error("Manual index is not allowed mixing with automatic index!"_cuqv);
                    ++next_index;
                }
                else
                {
                    if(next_index > 0)
                        throw format_error("Automatic index is not allowed mixing with manual index!"_cuqv);
                    const auto [ last, error ] = std::from_chars(index_run.c_str(), index_run.last(), current_index);
                    if(last != index_run.last())
                        throw format_error("Invalid format index [{}]!"_cuqv, index_run);
                    next_index = manual_index;
                }
                if(current_index < 0)
                    throw format_error("Invalid format index [{}]: Index should not be negative!"_cuqv, current_index);
                if(current_index >= argument_count)
                    throw format_error("Invalid format index [{}]: Index should be less than count of argument [{}]!"_cuqv, current_index, argument_count);
                result += arguments[current_index].format(specification);
            }
                break;
            case format_view::run_type::ending:
                // Unreachable
                break;
            }
        }
        return result;
    }
}

template<class Format, class...Args>
[[nodiscard]] codeunit_sequence format(const Format& format_literal, Args&&...args)
{
    return details::format(details::format_view{ details::view_sequence(format_literal) }, args...);
}

// code-region-start: formatter specializations for built-in types

namespace details
{
    codeunit_sequence OPEN_STRING_API format_integer(const i64& value, const codeunit_sequence_view& specification);
    codeunit_sequence OPEN_STRING_API format_float(const float& value, const codeunit_sequence_view& specification);
}

template<> 
struct formatter<i32>
{
    static codeunit_sequence format_argument(const i32& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct formatter<i64>
{
    static codeunit_sequence format_argument(const i64& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct formatter<float>
{
    static codeunit_sequence format_argument(const float& value, const codeunit_sequence_view& specification)
    {
        return details::format_float(value, specification);
    }
};

template<> 
struct formatter<index_interval>
{
    static codeunit_sequence format_argument(const index_interval& value, const codeunit_sequence_view& specification)
    {
        if(value.is_empty())
            return codeunit_sequence{ "∅"_cuqv };
        const index_interval::bound lower = value.get_lower_bound();
        codeunit_sequence lower_part;
        switch(lower.type)
        {
        case index_interval::bound::inclusion::inclusive:
            lower_part = "["_cuqv;
            lower_part += details::format_integer(lower.value, { });
            break;
        case index_interval::bound::inclusion::exclusive:
            lower_part = "("_cuqv;
            lower_part += details::format_integer(lower.value, { });
            break;
        case index_interval::bound::inclusion::infinity:
            lower_part = "(-∞"_cuqv;
            break;
        }
        const index_interval::bound upper = value.get_upper_bound();
        codeunit_sequence upper_part;
        switch(upper.type)
        {
        case index_interval::bound::inclusion::inclusive:
            upper_part = details::format_integer(upper.value, { });
            upper_part += "]"_cuqv;
            break;
        case index_interval::bound::inclusion::exclusive:
            upper_part = details::format_integer(upper.value, { });
            upper_part += ")"_cuqv;
            break;
        case index_interval::bound::inclusion::infinity:
            upper_part = "+∞)"_cuqv;
            break;
        }
        return format("{},{}"_cuqv, lower_part, upper_part);
    }
};

template<> 
struct formatter<const char*>
{
    static codeunit_sequence format_argument(const char* value, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<size_t N> 
struct formatter<char[N]>
{
    static codeunit_sequence format_argument(const char (&value)[N], const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<> 
struct formatter<codeunit_sequence_view>
{
    static codeunit_sequence format_argument(const codeunit_sequence_view& value, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<> 
struct formatter<codeunit_sequence>
{
    static const codeunit_sequence& format_argument(const codeunit_sequence& value, const codeunit_sequence_view& specification)
    {
        return value;
    }
};

template<>
struct formatter<std::nullptr_t>
{
    static codeunit_sequence format_argument(std::nullptr_t, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{"nullptr"_cuqv};
    }
};

template<class T> 
struct formatter<T*>
{
    static codeunit_sequence format_argument(const T* value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(reinterpret_cast<i64>(value), "#016x"_cuqv);
    }
};

// code-region-end: formatter specializations for built-in types

template<class Arg1, class...Args>
format_error::format_error(const codeunit_sequence_view& message_format, Arg1&& arg1, Args&&...args)
    : std::runtime_error(format(message_format, arg1, args...).c_str())
{ }

template <class T>
codeunit_sequence formatter<T>::format_argument(const T& value, const codeunit_sequence_view& specification)
{
    constexpr i32 size = sizeof(T);
    const auto reader = reinterpret_cast<const u8*>(&value);
    codeunit_sequence raw;
    for(i32 i = 0; i < size; ++i)
    {
        const u8 memory = reader[i];
        raw += " "_cuqv;
        raw += details::format_integer(memory, "02x"_cuqv);
    }
    
    if(specification == "r"_cuqv)   // output raw memory bytes
        return format("[Undefined type (raw:{})]"_cuqv, raw);

    const codeunit_sequence message = format("Undefined format with raw memory bytes:{}!"_cuqv, raw);
    throw format_error(message.view());
}

OPEN_STRING_NS_END

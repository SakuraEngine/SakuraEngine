// OpenString - formatting library
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once

#include <array>
#include <cmath>
#include <charconv>
#include "codeunit_sequence.h"
#include "utils/demangle.hpp"

OPEN_STRING_NS_BEGIN

template<class Format, class...Args>
[[nodiscard]] codeunit_sequence format(const Format& format_mold_literal, Args&&...args);

template<class T>
struct argument_formatter
{
    static codeunit_sequence produce(const T& value, const codeunit_sequence_view& specification);
};

namespace details
{
    class format_mold_view
    {
    public:
    // code-region-start: constructors

        explicit constexpr format_mold_view(const codeunit_sequence_view& format_mold) noexcept
            : format_mold_(format_mold)
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
                : format_mold(v)
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
                return { this->type, this->format_mold.subview(this->range) };
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
                const i32 index = this->format_mold.index_any_of(OSTR_UTF8("{}"_cuqv), { '[', from, '~' });
                if(index == index_invalid)
                    return { run_type::plain_text, { '[', from, '~' } };
                if(from != index)
                    return { run_type::plain_text, { '[', from, index, ')' } };
                if(format_mold[index] == format_mold[index+1])
                    return { run_type::escaped_brace, { '[', index, index + 1, ']' } };
                OPEN_STRING_CHECK(format_mold[index] != '}', OSTR_UTF8("Unclosed right brace is not allowed!"))
                const i32 index_next = this->format_mold.index_any_of(OSTR_UTF8("{}"_cuqv), { '(', index, '~' });
                OPEN_STRING_CHECK(index_next != index_invalid && format_mold[index_next] != '{', OSTR_UTF8("Unclosed left brace is not allowed!"))
                return { run_type::formatter, { '[', index, index_next, ']' } };
            }
            
            codeunit_sequence_view format_mold;
            run_type type = run_type::ending;
            index_interval range = index_interval::empty();
        };
        
        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return const_iterator{ this->format_mold_ }.initialize();
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return const_iterator{ this->format_mold_ };
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
        codeunit_sequence_view format_mold_;
    };

    template <class T>
    codeunit_sequence format_argument_value(const void* value, const codeunit_sequence_view specification)
    {
        return argument_formatter<T>::produce(*static_cast<const T*>(value), specification);
    }

    struct argument_value_package
    {
        using argument_value_formatter_type = codeunit_sequence(*)(const void* value, codeunit_sequence_view specification);

        const void* value;
        argument_value_formatter_type argument_value_formatter;

        template<class T>
        explicit constexpr argument_value_package(const T& v) noexcept
            : value(static_cast<const void*>(&v))
            , argument_value_formatter(format_argument_value<T>)
        { }

        [[nodiscard]] codeunit_sequence produce(const codeunit_sequence_view specification) const
        {
            return this->argument_value_formatter(this->value, specification);
        }
    };

    template<class...Args>
    codeunit_sequence produce_format(const format_mold_view& format_mold, const Args&...args)
    {
        constexpr i32 argument_count = sizeof...(Args);
        const std::array<argument_value_package, argument_count> arguments {{ argument_value_package{ args } ... }};

        constexpr i32 manual_index = -1;
        i32 next_index = 0;
        codeunit_sequence result;
        for(const auto [ type, run ] : format_mold)
        {
            switch (type) 
            {
            case format_mold_view::run_type::plain_text:
                result += run;
                break;
            case format_mold_view::run_type::escaped_brace:
                result += run[0];
                break;
            case format_mold_view::run_type::formatter:
            {    
                const auto [ index_run, specification ] = run.subview({ '[', 1, -2, ']' }).split(OSTR_UTF8(":"_cuqv));
                i32 current_index = next_index;
                if (index_run.is_empty())
                {
                    OPEN_STRING_CHECK(next_index != manual_index, OSTR_UTF8("Manual index is not allowed mixing with automatic index!"))
                    ++next_index;
                }
                else
                {
                    OPEN_STRING_CHECK(next_index <= 0, OSTR_UTF8("Automatic index is not allowed mixing with manual index!"))
                    [[maybe_unused]] const auto [ last, error ] = 
                        std::from_chars(index_run.c_str(), (const char*)index_run.last(), current_index);
                    OPEN_STRING_CHECK(last == (const char*)index_run.last(), OSTR_UTF8("Invalid format index [{}]!"), index_run)
                    next_index = manual_index;
                }
                OPEN_STRING_CHECK(current_index >= 0, OSTR_UTF8("Invalid format index [{}]: Index should not be negative!"), current_index)
                OPEN_STRING_CHECK(current_index < argument_count, OSTR_UTF8("Invalid format index [{}]: Index should be less than count of argument [{}]!"), current_index, argument_count)
                result += arguments[current_index].produce(specification);
            }
                break;
            case format_mold_view::run_type::ending:
                // Unreachable
                break;
            }
        }
        return result;
    }
}

template<class Format, class...Args>
[[nodiscard]] codeunit_sequence format(const Format& format_mold_literal, Args&&...args)
{
    return details::produce_format(details::format_mold_view{ details::view_sequence(format_mold_literal) }, args...);
}

// code-region-start: formatter specializations for built-in types

namespace details
{
    codeunit_sequence OPEN_STRING_API format_integer(const i64& value, const codeunit_sequence_view& specification);
    codeunit_sequence OPEN_STRING_API format_float(const float& value, const codeunit_sequence_view& specification);
}

template<> 
struct argument_formatter<i8>
{
    static codeunit_sequence produce(const i8& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

#if defined(_MSC_VER)

template<> 
struct argument_formatter<long>
{
    static codeunit_sequence produce(const long& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<unsigned long>
{
    static codeunit_sequence produce(const unsigned long& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<long double>
{
    static codeunit_sequence produce(const long double& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

#endif

#if defined(__linux__)

template<> 
struct argument_formatter<long long int>
{
    static codeunit_sequence produce(const long long int& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<<unsigned long long int>
{
    static codeunit_sequence produce(const <unsigned long long int& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

#endif

template<> 
struct argument_formatter<u8>
{
    static codeunit_sequence produce(const u8& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<i16>
{
    static codeunit_sequence produce(const i16& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<u16>
{
    static codeunit_sequence produce(const u16& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<i32>
{
    static codeunit_sequence produce(const i32& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<u32>
{
    static codeunit_sequence produce(const u32& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<i64>
{
    static codeunit_sequence produce(const i64& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<u64>
{
    static codeunit_sequence produce(const u64& value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(value, specification);
    }
};

template<> 
struct argument_formatter<float>
{
    static codeunit_sequence produce(const float& value, const codeunit_sequence_view& specification)
    {
        return details::format_float(value, specification);
    }
};

template<> 
struct argument_formatter<index_interval>
{
    static codeunit_sequence produce(const index_interval& value, const codeunit_sequence_view& specification)
    {
        if(value.is_empty())
            return codeunit_sequence{ OSTR_UTF8("∅"_cuqv) };
        const index_interval::bound lower = value.get_lower_bound();
        codeunit_sequence lower_part;
        switch(lower.type)
        {
        case index_interval::bound::inclusion::inclusive:
            lower_part = OSTR_UTF8("["_cuqv);
            lower_part += details::format_integer(lower.value, { });
            break;
        case index_interval::bound::inclusion::exclusive:
            lower_part = OSTR_UTF8("("_cuqv);
            lower_part += details::format_integer(lower.value, { });
            break;
        case index_interval::bound::inclusion::infinity:
            lower_part = OSTR_UTF8("(-∞"_cuqv);
            break;
        }
        const index_interval::bound upper = value.get_upper_bound();
        codeunit_sequence upper_part;
        switch(upper.type)
        {
        case index_interval::bound::inclusion::inclusive:
            upper_part = details::format_integer(upper.value, { });
            upper_part += OSTR_UTF8("]"_cuqv);
            break;
        case index_interval::bound::inclusion::exclusive:
            upper_part = details::format_integer(upper.value, { });
            upper_part += OSTR_UTF8(")"_cuqv);
            break;
        case index_interval::bound::inclusion::infinity:
            upper_part = OSTR_UTF8("+∞)"_cuqv);
            break;
        }
        return format(OSTR_UTF8("{},{}"_cuqv), lower_part, upper_part);
    }
};

template<> 
struct argument_formatter<const ochar8_t*>
{
    static codeunit_sequence produce(const ochar8_t* value, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<size_t N> 
struct argument_formatter<ochar8_t[N]>
{
    static codeunit_sequence produce(const ochar8_t (&value)[N], const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<> 
struct argument_formatter<codeunit_sequence_view>
{
    static codeunit_sequence produce(const codeunit_sequence_view& value, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{value};
    }
};

template<> 
struct argument_formatter<codeunit_sequence>
{
    static const codeunit_sequence& produce(const codeunit_sequence& value, const codeunit_sequence_view& specification)
    {
        return value;
    }
};

template<>
struct argument_formatter<std::nullptr_t>
{
    static codeunit_sequence produce(std::nullptr_t, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{OSTR_UTF8("nullptr"_cuqv)};
    }
};

template<class T> 
struct argument_formatter<T*>
{
    static codeunit_sequence produce(const T* value, const codeunit_sequence_view& specification)
    {
        return details::format_integer(reinterpret_cast<i64>(value), OSTR_UTF8("#016x"_cuqv));
    }
};

// code-region-end: formatter specializations for built-in types

template <class T>
codeunit_sequence argument_formatter<T>::produce(const T& value, const codeunit_sequence_view& specification)
{
    constexpr i32 size = sizeof(T);
    const auto reader = reinterpret_cast<const u8*>(&value);
    codeunit_sequence raw;
    for(i32 i = 0; i < size; ++i)
    {
        const u8 memory = reader[i];
        raw += OSTR_UTF8(" "_cuqv);
        raw += details::format_integer(memory, OSTR_UTF8("02x"_cuqv));
    }
    
    if(specification == OSTR_UTF8("r"_cuqv))   // output raw memory bytes
        return format(OSTR_UTF8("[Undefined type (raw:{})]"_cuqv), raw);

    const auto t = skr::demangle<T>();
    OPEN_STRING_CHECK(false, OSTR_UTF8("Undefined format with raw memory bytes:{}, with cplusplus type {}!"), raw, (const ochar8_t*)t.c_str());
    return { };
}

OPEN_STRING_NS_END

#pragma once

#include <array>
#include <cmath>
#include <charconv>
#include "common/platforms.h"
#include "common/definitions.h"
#include "codeunit_sequence.h"

namespace ostr
{
    namespace details
    {
        codeunit_sequence OPEN_STRING_API format_integer(const u64& value, const codeunit_sequence_view& specification);
        codeunit_sequence OPEN_STRING_API format_integer(const i64& value, const codeunit_sequence_view& specification);
        codeunit_sequence OPEN_STRING_API format_float(const f64& value, const codeunit_sequence_view& specification);
    }
    
    template<class Format, class...Args>
    [[nodiscard]] codeunit_sequence format(const Format& format_mold_literal, const Args&...args);

    template<class T, typename=void>
    struct argument_formatter
    {
        static codeunit_sequence produce(const T& value, const codeunit_sequence_view& specification)
        {
            constexpr u64 size = sizeof(T);
            const auto reader = reinterpret_cast<const byte*>(&value);
            codeunit_sequence raw;
            for(u64 i = 0; i < size; ++i)
            {
                if(!raw.is_empty())
                    raw += u8" "_cuqv;
                const byte memory = reader[i];
                raw += details::format_integer(static_cast<u64>(memory), u8"02x"_cuqv);
            }
    
            if(specification == u8"r"_cuqv)   // output raw memory bytes
                return raw;

            OPEN_STRING_CHECK(false, u8"Undefined format with raw memory bytes:{}!", raw);
            return { };
        }
    };

    namespace details
    {
        class format_mold_view
        {
        public:
            // code-region-start: constructors

            explicit constexpr format_mold_view(codeunit_sequence_view format_mold) noexcept
                : format_mold_(std::move(format_mold))
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

                explicit constexpr const_iterator(codeunit_sequence_view v) noexcept
                    : format_mold(std::move(v))
                { }

                constexpr const_iterator& initialize() noexcept
                {
                    const auto [ first_type, first_from, first_size ] = this->parse_run(0);
                    this->type = first_type;
                    this->from = first_from;
                    this->size = first_size;
                    return *this;
                }

                [[nodiscard]] constexpr std::tuple<run_type, codeunit_sequence_view> operator*() const noexcept
                {
                    return { this->type, this->format_mold.subview(this->from, this->size) };
                }

                constexpr const_iterator& operator++()
                {
                    if(this->from == global_constant::INDEX_INVALID)
                    {
                        this->type = run_type::ending;
                        this->from = global_constant::INDEX_INVALID;
                        this->size = SIZE_MAX;
                        return *this;
                    }
                    const auto [ next_type, next_from, next_size ] = this->parse_run(this->from + this->size);
                    this->type = next_type;
                    this->from = next_from;
                    this->size = next_size;
                    return *this;
                }

                constexpr const_iterator operator++(int) noexcept
                {
                    const_iterator tmp = *this;
                    ++*this;
                    return tmp;
                }

                [[nodiscard]] constexpr bool operator==(const const_iterator& rhs) const noexcept
                {
                    return this->type == rhs.type && this->from == rhs.from && this->size == rhs.size;
                }

                [[nodiscard]] constexpr bool operator!=(const const_iterator& rhs) const noexcept
                {
                    return !(*this == rhs);
                }

                struct result_parse_run
                {
                    run_type type = run_type::plain_text;
                    u64 from = global_constant::INDEX_INVALID;
                    u64 size = SIZE_MAX;
                };
                [[nodiscard]] constexpr result_parse_run parse_run(const u64 from_index) const
                {
                    const u64 index = this->format_mold.index_of_any(u8"{}"_cuqv, from_index);
                    if(index == global_constant::INDEX_INVALID)
                    {
                        if(from_index == this->format_mold.size())
                            return { run_type::ending };
                        return { run_type::plain_text, from_index, this->format_mold.size() - from_index };
                    }
                    if(from_index != index)
                        return { run_type::plain_text, from_index, index - from_index };
                    if(format_mold.read_at(index) == format_mold.read_at(index + 1))
                        return { run_type::escaped_brace, index, 2 };
                    OPEN_STRING_CHECK(format_mold.read_at(index) != u8'}', u8"Unclosed right brace is not allowed!");
                    const u64 index_next = this->format_mold.index_of_any(u8"{}"_cuqv, index + 1);
                    OPEN_STRING_CHECK(index_next != global_constant::INDEX_INVALID && format_mold.read_at(index_next) != u8'{', u8"Unclosed left brace is not allowed!");
                    return { run_type::formatter, index, index_next - index + 1 };
                }
            
                codeunit_sequence_view format_mold;
                run_type type = run_type::ending;
                u64 from = global_constant::INDEX_INVALID;
                u64 size = SIZE_MAX;
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
            codeunit_sequence_view format_mold_{ };
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

            [[nodiscard]] codeunit_sequence produce(const codeunit_sequence_view& specification) const
            {
                return this->argument_value_formatter(this->value, specification);
            }
        };

        template<class...Args>
        codeunit_sequence produce_format(const format_mold_view& format_mold, const Args&...args)
        {
            constexpr u64 argument_count = sizeof...(Args);
            const std::array<argument_value_package, argument_count> arguments {{ argument_value_package{ args } ... }};

            enum class indexing_type : u8
            {
                unknown,
                manual,
                automatic
            };
            indexing_type index_type = indexing_type::unknown;
            u64 next_index = 0;
            codeunit_sequence result;
            for(const auto [ type, run ] : format_mold)
            {
                switch (type) 
                {
                case format_mold_view::run_type::plain_text:
                    result += run;
                    break;
                case format_mold_view::run_type::escaped_brace:
                    result += run.read_at(0);
                    break;
                case format_mold_view::run_type::formatter:
                    {    
                        const auto [ index_run, specification ] = run.subview(1, run.size() - 2).split(u8":"_cuqv);
                        u64 current_index = next_index;
                        if (index_run.is_empty())
                        {
                            OPEN_STRING_CHECK(index_type != indexing_type::manual, u8"Manual index is not allowed mixing with automatic index!");
                            index_type = indexing_type::automatic;
                            ++next_index;
                        }
                        else
                        {
                            OPEN_STRING_CHECK(index_type != indexing_type::automatic, u8"Automatic index is not allowed mixing with manual index!");
                            index_type = indexing_type::manual;
                            [[maybe_unused]] const auto [ last, error ] = 
                                std::from_chars((const char*)index_run.data(), (const char*)index_run.cend().data(), current_index);
                            OPEN_STRING_CHECK(last == (const char*)index_run.cend().data(), u8"Invalid format index [{}]!", index_run);
                        }
                        OPEN_STRING_CHECK(current_index < argument_count, u8"Invalid format index [{}]: Index should be less than count of argument [{}]!", current_index, argument_count);
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
    [[nodiscard]] codeunit_sequence format(const Format& format_mold_literal, const Args&...args)
    {
        return details::produce_format(details::format_mold_view{ details::view_sequence(format_mold_literal) }, args...);
    }

    // code-region-start: formatter specializations for built-in types

    template<class T>
    struct argument_formatter<T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
    {
        static codeunit_sequence produce(const T& value, const codeunit_sequence_view& specification)
        {
            return details::format_integer(static_cast<i64>(value), specification);
        }
    };

    template<class T>
    struct argument_formatter<T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
    {
        static codeunit_sequence produce(const T& value, const codeunit_sequence_view& specification)
        {
            return details::format_integer(static_cast<u64>(value), specification);
        }
    };

    template<class T> 
    struct argument_formatter<T, std::enable_if_t<std::is_floating_point_v<T>>>
    {
        static codeunit_sequence produce(const T& value, const codeunit_sequence_view& specification)
        {
            return details::format_float(static_cast<f64>(value), specification);
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
            return codeunit_sequence{u8"nullptr"_cuqv};
        }
    };

    template<class T> 
    struct argument_formatter<T*>
    {
        static codeunit_sequence produce(const T* value, const codeunit_sequence_view& specification)
        {
            return details::format_integer(reinterpret_cast<i64>(value), u8"#016x"_cuqv);
        }
    };

    // code-region-end: formatter specializations for built-in types
}

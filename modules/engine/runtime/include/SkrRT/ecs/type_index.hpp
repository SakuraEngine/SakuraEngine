#pragma once
#include "SkrRT/ecs/sugoi.h" // IWYU pragma: export

namespace sugoi
{
class type_index_t
{
    static_assert(sizeof(TIndex) * 8 == 32, "TIndex should be 32 bits");
    union
    {
        TIndex value;
        struct
        {
            TIndex id : 28;
            TIndex pin : 1;
            TIndex buffer : 1;
            TIndex chunk: 1;
            TIndex tag : 1;
        };
    };

public:
    constexpr TIndex index() const noexcept { return id; }
    constexpr bool is_pod() const noexcept { return value == id; }
    constexpr bool is_buffer() const noexcept { return buffer; }
    constexpr bool is_tag() const noexcept { return tag; }
    constexpr bool is_pinned() const noexcept { return pin; }
    constexpr bool is_chunk() const noexcept { return chunk; }

    constexpr type_index_t() noexcept
        : value(kInvalidTypeIndex)
    {
    }
    constexpr type_index_t(TIndex value) noexcept
        : value(value)
    {
    }
    constexpr type_index_t(TIndex a, bool pin, bool buffer, bool tag, bool chunk) noexcept
        : id(a)
        , pin(pin)
        , buffer(buffer)
        , chunk(chunk)
        , tag(tag)
    {
    }

    constexpr operator TIndex() const { return value; }
};
} // namespace sugoi
#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include <string>

namespace skr::container
{
template <typename TS>
struct U8StringView {
    // basic
    using DataType = skr_char8;
    using SizeType = TS;

    // data ref
    using CDataRef = StringDataRef<DataType, SizeType, true>;

    // helper
    using CharTraits               = std::char_traits<DataType>;
    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & dtor
    constexpr U8StringView();
    constexpr U8StringView(const DataType* str, SizeType len);
    constexpr U8StringView(const DataType* begin, const DataType* end);
    template <size_t N>
    constexpr U8StringView(const DataType (&arr)[N]);
    template <LinearMemoryContainer U>
    constexpr U8StringView(U&& container);
    constexpr ~U8StringView();

    // copy & move
    constexpr U8StringView(const U8StringView& other);
    constexpr U8StringView(U8StringView&& other) noexcept;

    // assign & move assign
    constexpr U8StringView& operator=(const U8StringView& rhs);
    constexpr U8StringView& operator=(U8StringView&& rhs) noexcept;

    // compare
    constexpr bool operator==(const U8StringView& rhs) const;
    constexpr bool operator!=(const U8StringView& rhs) const;
    constexpr bool operator==(const DataType* rhs) const;
    constexpr bool operator!=(const DataType* rhs) const;

    // getter
    constexpr const DataType* data() const;
    constexpr SizeType        size() const;
    constexpr bool            is_empty() const;

    // str getter
    constexpr SizeType length_buffer() const;
    constexpr SizeType length_text() const;

    // index & modify
    constexpr const DataType& at_buffer(SizeType index) const;
    constexpr const DataType& last_buffer(SizeType index) const;
    constexpr UTF8Seq         at_text(SizeType index) const;
    constexpr UTF8Seq         last_text(SizeType index) const;

    // sub views
    constexpr U8StringView first(SizeType count) const;
    constexpr U8StringView last(SizeType count) const;
    constexpr U8StringView subview(SizeType start, SizeType count = npos) const;

    // find
    constexpr CDataRef find(const U8StringView& pattern) const;
    constexpr CDataRef find_last(const U8StringView& pattern) const;

    // contains & count
    constexpr bool     contains(const U8StringView& pattern) const;
    constexpr SizeType count(const U8StringView& pattern) const;

    // starts & ends
    constexpr bool starts_with(const U8StringView& pattern) const;
    constexpr bool ends_with(const U8StringView& pattern) const;

    // remove prefix & suffix
    constexpr U8StringView remove_prefix(const U8StringView& pattern) const;
    constexpr U8StringView remove_suffix(const U8StringView& pattern) const;

    // trim
    constexpr U8StringView trim(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_start(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_end(const U8StringView& characters = u8" \t") const;

    // split
    template <typename Buffer>
    constexpr SizeType split(Buffer& out, const U8StringView& delimiter, SizeType limit = npos) const;

    // text index
    SizeType buffer_index_to_text(SizeType index) const;
    SizeType text_index_to_buffer(SizeType index) const;

    // TODO. as int/float

private:
    DataType* _data = nullptr;
    SizeType  _size = 0;
};
} // namespace skr::container

namespace skr::container
{
}
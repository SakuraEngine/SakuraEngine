#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include <string>
#include "SkrBase/misc/debug.h"

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
    constexpr U8StringView(const DataType* str);
    constexpr U8StringView(const DataType* str, SizeType len);
    constexpr U8StringView(const DataType* begin, const DataType* end);
    template <size_t N>
    constexpr U8StringView(const DataType (&arr)[N]);
    template <LinearMemoryContainer U>
    constexpr U8StringView(const U& container);
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

    // validator
    constexpr bool is_valid_index(SizeType index) const;
    constexpr bool is_valid_ptr(const DataType* ptr) const;

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
    // TODO. single code point
    constexpr CDataRef find(const U8StringView& pattern) const;
    constexpr CDataRef find_last(const U8StringView& pattern) const;

    // contains & count
    // TODO. single code point
    constexpr bool     contains(const U8StringView& pattern) const;
    constexpr SizeType count(const U8StringView& pattern) const;

    // starts & ends
    // TODO. single code point
    constexpr bool starts_with(const U8StringView& prefix) const;
    constexpr bool ends_with(const U8StringView& suffix) const;

    // remove prefix & suffix
    // TODO. single code point
    constexpr U8StringView remove_prefix(const U8StringView& prefix) const;
    constexpr U8StringView remove_suffix(const U8StringView& suffix) const;

    // trim
    // TODO. single code point
    constexpr U8StringView trim(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_start(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_end(const U8StringView& characters = u8" \t") const;

    // trim invalid
    constexpr U8StringView trim_invalid() const;
    constexpr U8StringView trim_invalid_start() const;
    constexpr U8StringView trim_invalid_end() const;

    // split
    // TODO. single code point
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
// ctor & dtor
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView() = default;
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView(const DataType* str)
    : _data(const_cast<DataType*>(str))
    , _size(CharTraits::length(str))
{
}
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView(const DataType* str, SizeType len)
    : _data(const_cast<DataType*>(str))
    , _size(len)
{
}
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView(const DataType* begin, const DataType* end)
    : _data(const_cast<DataType*>(begin))
    , _size(end - begin)
{
}
template <typename TS>
template <size_t N>
inline constexpr U8StringView<TS>::U8StringView(const DataType (&arr)[N])
    : _data(const_cast<DataType*>(arr))
    , _size(N - 1)
{
}
template <typename TS>
template <LinearMemoryContainer U>
inline constexpr U8StringView<TS>::U8StringView(const U& container)
    : _data(const_cast<DataType*>(container.data()))
    , _size(container.size())
{
}
template <typename TS>
inline constexpr U8StringView<TS>::~U8StringView() = default;

// copy & move
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView(const U8StringView& other) = default;
template <typename TS>
inline constexpr U8StringView<TS>::U8StringView(U8StringView&& other) noexcept = default;

// assign & move assign
template <typename TS>
inline constexpr U8StringView<TS>& U8StringView<TS>::operator=(const U8StringView& rhs) = default;
template <typename TS>
inline constexpr U8StringView<TS>& U8StringView<TS>::operator=(U8StringView&& rhs) noexcept = default;

// compare
template <typename TS>
inline constexpr bool U8StringView<TS>::operator==(const U8StringView& rhs) const
{
    if (_size != rhs._size)
    {
        return false;
    }
    else
    {
        return CharTraits::compare(_data, rhs._data, _size) == 0;
    }
}
template <typename TS>
inline constexpr bool U8StringView<TS>::operator!=(const U8StringView& rhs) const
{
    return !(*this == rhs);
}
template <typename TS>
inline constexpr bool U8StringView<TS>::operator==(const DataType* rhs) const
{
    auto rhs_size = CharTraits::length(rhs);
    if (_size != rhs_size)
    {
        return false;
    }
    else
    {
        return CharTraits::compare(_data, rhs, _size) == 0;
    }
}
template <typename TS>
inline constexpr bool U8StringView<TS>::operator!=(const DataType* rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename TS>
inline constexpr const U8StringView<TS>::DataType* U8StringView<TS>::data() const
{
    return _data;
}
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::size() const
{
    return _size;
}
template <typename TS>
inline constexpr bool U8StringView<TS>::is_empty() const
{
    return _size == 0;
}

// str getter
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::length_buffer() const
{
    return _size;
}
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::length_text() const
{
    if (_size == 0)
    {
        return 0;
    }

    SizeType result  = 0;
    SizeType cur_idx = 0;
    while (cur_idx < _size)
    {
        auto seq_len = utf8_seq_len(_data[cur_idx]);
        cur_idx += seq_len == 0 ? 1 : seq_len;
        ++result;
    }
    return result;
}

// validator
template <typename TS>
inline constexpr bool U8StringView<TS>::is_valid_index(SizeType index) const
{
    return index >= 0 && index < _size;
}
template <typename TS>
inline constexpr bool U8StringView<TS>::is_valid_ptr(const DataType* ptr) const
{
    return ptr >= _data && ptr < (_data + _size);
}

// index & modify
template <typename TS>
inline constexpr const U8StringView<TS>::DataType& U8StringView<TS>::at_buffer(SizeType index) const
{
    SKR_ASSERT(!is_empty() && "undefined behavior accessing an empty string view");
    SKR_ASSERT(is_valid_index(index) && "undefined behavior accessing out of bounds");

    return _data[index];
}
template <typename TS>
inline constexpr const U8StringView<TS>::DataType& U8StringView<TS>::last_buffer(SizeType index) const
{
    index = _size - index - 1;
    SKR_ASSERT(!is_empty() && "undefined behavior accessing an empty string view");
    SKR_ASSERT(is_valid_index(index) && "undefined behavior accessing out of bounds");
    return _data[index];
}
template <typename TS>
inline constexpr UTF8Seq U8StringView<TS>::at_text(SizeType index) const
{
    SKR_ASSERT(!is_empty() && "undefined behavior accessing an empty string view");
    SKR_ASSERT(is_valid_index(index) && "undefined behavior accessing out of bounds");
    uint64_t seq_index;
    return utf8_parse_seq(_data, _size, index, seq_index);
}
template <typename TS>
inline constexpr UTF8Seq U8StringView<TS>::last_text(SizeType index) const
{
    index = _size - index - 1;
    SKR_ASSERT(!is_empty() && "undefined behavior accessing an empty string view");
    SKR_ASSERT(is_valid_index(index) && "undefined behavior accessing out of bounds");
    uint64_t seq_index;
    return utf8_parse_seq(_data, _size, index, seq_index);
}

// sub views
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::first(SizeType count) const
{
    SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
    return { _data, count };
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::last(SizeType count) const
{
    SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
    return { _data + _size - count, count };
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::subview(SizeType start, SizeType count) const
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");
    return { _data + start, count == npos ? size() - start : count };
}

// find
template <typename TS>
inline constexpr U8StringView<TS>::CDataRef U8StringView<TS>::find(const U8StringView& pattern) const
{
    if (is_empty() || pattern.is_empty() || size() < pattern.size())
    {
        return {};
    }

    auto match_try = _data;
    auto match_end = _data + (_size - pattern.size()) + 1;
    while (true)
    {
        match_try = CharTraits::find(match_try, match_end - match_try, pattern.at_buffer(0));
        if (!match_try)
        {
            return {};
        }

        if (CharTraits::compare(match_try, pattern.data(), pattern.size()) == 0)
        {
            return { match_try, match_try - _data };
        }

        ++match_try;
    }
}
template <typename TS>
inline constexpr U8StringView<TS>::CDataRef U8StringView<TS>::find_last(const U8StringView& pattern) const
{
    if (is_empty() || pattern.is_empty() || size() < pattern.size())
    {
        return {};
    }

    auto match_try = _data + _size - pattern.size();
    while (true)
    {
        if (CharTraits::eq(*match_try, pattern.at_buffer(0)) &&
            CharTraits::compare(match_try, pattern.data(), pattern.size()) == 0)
        {
            return { match_try, match_try - _data };
        }

        if (match_try == _data)
        {
            break;
        }

        --match_try;
    }

    return {};
}

// contains & count
template <typename TS>
inline constexpr bool U8StringView<TS>::contains(const U8StringView& pattern) const
{
    return (bool)find(pattern);
}
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::count(const U8StringView& pattern) const
{
    auto     find_view{ *this };
    SizeType result = 0;
    while (true)
    {
        auto find_result = find_view.find(pattern);
        if (find_result)
        {
            ++result;
            find_view = find_view.subview(find_result.index() + pattern.size());
        }
        else
        {
            break;
        }
    }
    return result;
}

// starts & ends
template <typename TS>
inline constexpr bool U8StringView<TS>::starts_with(const U8StringView& prefix) const
{
    if (prefix.is_empty())
    {
        return true;
    }
    if (size() < prefix.size())
    {
        return false;
    }
    return CharTraits::compare(_data, prefix.data(), prefix.size()) == 0;
}
template <typename TS>
inline constexpr bool U8StringView<TS>::ends_with(const U8StringView& suffix) const
{
    if (suffix.is_empty())
    {
        return true;
    }
    if (size() < suffix.size())
    {
        return false;
    }

    return CharTraits::compare(_data + (_size - suffix.size()), suffix.data(), suffix.size()) == 0;
}

// remove prefix & suffix
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::remove_prefix(const U8StringView& prefix) const
{
    return starts_with(prefix) ? subview(prefix.size()) : *this;
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::remove_suffix(const U8StringView& suffix) const
{
    return ends_with(suffix) ? subview(0, size() - suffix.size()) : *this;
}

// trim
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim(const U8StringView& characters) const
{
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_start(const U8StringView& characters) const
{
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_end(const U8StringView& characters) const
{
}
} // namespace skr::container

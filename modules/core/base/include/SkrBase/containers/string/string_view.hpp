#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include "SkrBase/unicode/unicode_iterator.hpp"
#include "SkrBase/misc/debug.h"
#include <string>

namespace skr::container
{
template <typename TS>
struct U8StringView {
    // basic
    using DataType = skr_char8;
    using SizeType = TS;

    // data ref
    // using DataRef  = StringDataRef<DataType, SizeType, false>;
    using CDataRef = StringDataRef<DataType, SizeType, true>;

    // cursor & iterator
    // using Cursor  = UTF8Cursor<SizeType, false>;
    using CCursor = UTF8Cursor<SizeType, true>;
    // using Iter    = UTF8Iter<SizeType, false>;
    using CIter = UTF8Iter<SizeType, true>;
    // using IterInv  = UTF8IterInv<SizeType, true>;
    using CIterInv = UTF8IterInv<SizeType, true>;
    // using Range   = UTF8Range<SizeType, false>;
    using CRange = UTF8Range<SizeType, true>;
    // using RangeInv  = UTF8RangeInv<SizeType, true>;
    using CRangeInv = UTF8RangeInv<SizeType, true>;

    // other types
    using PartitionResult = StringPartitionResult<U8StringView>;

    // helper
    using CharTraits               = std::char_traits<DataType>;
    static constexpr SizeType npos = npos_of<SizeType>;

    // traits
    static_assert(std::is_same_v<DataType, skr_char8>, "U8StringView only supports char8_t");

    // ctor & dtor
    constexpr U8StringView();
    constexpr U8StringView(const DataType* str);
    constexpr U8StringView(const DataType* str, SizeType len);
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
    constexpr CDataRef find(const U8StringView& pattern) const;
    constexpr CDataRef find_last(const U8StringView& pattern) const;
    constexpr CDataRef find(const UTF8Seq& pattern) const;
    constexpr CDataRef find_last(const UTF8Seq& pattern) const;

    // contains & count
    constexpr bool     contains(const U8StringView& pattern) const;
    constexpr SizeType count(const U8StringView& pattern) const;
    constexpr bool     contains(const UTF8Seq& pattern) const;
    constexpr SizeType count(const UTF8Seq& pattern) const;

    // starts & ends
    constexpr bool starts_with(const U8StringView& prefix) const;
    constexpr bool ends_with(const U8StringView& suffix) const;
    constexpr bool starts_with(const UTF8Seq& prefix) const;
    constexpr bool ends_with(const UTF8Seq& suffix) const;

    // remove prefix & suffix
    constexpr U8StringView remove_prefix(const U8StringView& prefix) const;
    constexpr U8StringView remove_suffix(const U8StringView& suffix) const;
    constexpr U8StringView remove_prefix(const UTF8Seq& prefix) const;
    constexpr U8StringView remove_suffix(const UTF8Seq& suffix) const;

    // trim
    constexpr U8StringView trim(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_start(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim_end(const U8StringView& characters = u8" \t") const;
    constexpr U8StringView trim(const UTF8Seq& ch) const;
    constexpr U8StringView trim_start(const UTF8Seq& ch) const;
    constexpr U8StringView trim_end(const UTF8Seq& ch) const;

    // trim invalid
    constexpr U8StringView trim_invalid() const;
    constexpr U8StringView trim_invalid_start() const;
    constexpr U8StringView trim_invalid_end() const;

    // partition
    constexpr PartitionResult partition(const U8StringView& delimiter) const;

    // split
    template <CanAdd<const U8StringView<TS>&> Buffer>
    constexpr SizeType split(Buffer& out, const U8StringView& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <std::invocable<const U8StringView<TS>&> F>
    constexpr SizeType split_each(F&& func, const U8StringView& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <CanAdd<const U8StringView<TS>&> Buffer>
    constexpr SizeType split(Buffer& out, const UTF8Seq& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <std::invocable<const U8StringView<TS>&> F>
    constexpr SizeType split_each(F&& func, const UTF8Seq& delimiter, bool cull_empty = false, SizeType limit = npos) const;

    // text index
    constexpr SizeType buffer_index_to_text(SizeType index) const;
    constexpr SizeType text_index_to_buffer(SizeType index) const;

    // cursor & iter
    CCursor   cursor_begin() const;
    CCursor   cursor_end() const;
    CIter     iter() const;
    CIterInv  iter_inv() const;
    CRange    range() const;
    CRangeInv range_inv() const;

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
    else if (is_empty() || rhs.is_empty())
    {
        return is_empty() == rhs.is_empty();
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
    return is_empty() ? 0 : utf8_code_point_index(_data, _size, _size - 1) + 1;
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
    return UTF8Seq::ParseUTF8(_data, _size, index, seq_index);
}
template <typename TS>
inline constexpr UTF8Seq U8StringView<TS>::last_text(SizeType index) const
{
    index = _size - index - 1;
    SKR_ASSERT(!is_empty() && "undefined behavior accessing an empty string view");
    SKR_ASSERT(is_valid_index(index) && "undefined behavior accessing out of bounds");
    uint64_t seq_index;
    return UTF8Seq::ParseUTF8(_data, _size, index, seq_index);
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

    const DataType* match_try = _data;
    auto            match_end = _data + (_size - pattern.size()) + 1;
    while (true)
    {
        match_try = CharTraits::find(match_try, match_end - match_try, pattern.at_buffer(0));
        if (!match_try)
        {
            return {};
        }

        if (CharTraits::compare(match_try, pattern.data(), pattern.size()) == 0)
        {
            return { match_try, static_cast<SizeType>(match_try - _data) };
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

    const DataType* match_try = _data + _size - pattern.size();
    while (true)
    {
        if (CharTraits::eq(*match_try, pattern.at_buffer(0)) &&
            CharTraits::compare(match_try, pattern.data(), pattern.size()) == 0)
        {
            return { match_try, static_cast<SizeType>(match_try - _data) };
        }

        if (match_try == _data)
        {
            break;
        }

        --match_try;
    }

    return {};
}
template <typename TS>
inline constexpr U8StringView<TS>::CDataRef U8StringView<TS>::find(const UTF8Seq& pattern) const
{
    if (pattern.is_valid())
    {
        // FIXME. maybe optimize in view find or just use uniform find is better?
        if (pattern.len == 1)
        {
            if (auto found = CharTraits::find(_data, _size, pattern.data[0]))
            {
                return { found, static_cast<SizeType>(found - _data) };
            }
            else
            {
                return {};
            }
        }
        else
        {
            return find(U8StringView{ &pattern.data[0], pattern.len });
        }
    }
    else
    {
        return {};
    }
}
template <typename TS>
inline constexpr U8StringView<TS>::CDataRef U8StringView<TS>::find_last(const UTF8Seq& pattern) const
{
    if (pattern.is_valid())
    {
        return find_last(U8StringView{ &pattern.data[0], pattern.len });
    }
    else
    {
        return {};
    }
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
template <typename TS>
inline constexpr bool U8StringView<TS>::contains(const UTF8Seq& pattern) const
{
    return (bool)find(pattern);
}
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::count(const UTF8Seq& pattern) const
{
    if (pattern.is_valid())
    {
        return count(U8StringView{ &pattern.data[0], pattern.len });
    }
    else
    {
        return 0;
    }
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
template <typename TS>
inline constexpr bool U8StringView<TS>::starts_with(const UTF8Seq& prefix) const
{
    if (prefix.is_valid())
    {
        return starts_with(U8StringView{ &prefix.data[0], prefix.len });
    }
    else
    {
        return false;
    }
}
template <typename TS>
inline constexpr bool U8StringView<TS>::ends_with(const UTF8Seq& suffix) const
{
    if (suffix.is_valid())
    {
        return ends_with(U8StringView{ &suffix.data[0], suffix.len });
    }
    else
    {
        return false;
    }
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
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::remove_prefix(const UTF8Seq& prefix) const
{
    if (prefix.is_valid())
    {
        return remove_prefix(U8StringView{ &prefix.data[0], prefix.len });
    }
    else
    {
        return *this;
    }
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::remove_suffix(const UTF8Seq& suffix) const
{
    if (suffix.is_valid())
    {
        return remove_suffix(U8StringView{ &suffix.data[0], suffix.len });
    }
    else
    {
        return *this;
    }
}

// trim
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim(const U8StringView& characters) const
{
    return trim_start(characters).trim_end(characters);
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_start(const U8StringView& characters) const
{
    if (is_empty())
    {
        return {};
    }
    if (characters.is_empty())
    {
        return { *this };
    }

    for (auto cursor = cursor_begin(); !cursor.reach_end(); cursor.move_next())
    {
        if (!characters.contains(cursor.ref()))
        {
            return this->subview(cursor.index());
        }
    }
    return {};
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_end(const U8StringView& characters) const
{
    if (is_empty())
    {
        return {};
    }
    if (characters.is_empty())
    {
        return { *this };
    }

    for (auto cursor = cursor_end(); !cursor.reach_begin(); cursor.move_prev())
    {
        if (!characters.contains(cursor.ref()))
        {
            return this->subview(0, cursor.index() + cursor.seq_len());
        }
    }
    return {};
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim(const UTF8Seq& ch) const
{
    if (ch.is_valid())
    {
        return trim(U8StringView{ &ch.data[0], ch.len });
    }
    else
    {
        return { *this };
    }
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_start(const UTF8Seq& ch) const
{
    if (ch.is_valid())
    {
        return trim_start(U8StringView{ &ch.data[0], ch.len });
    }
    else
    {
        return { *this };
    }
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_end(const UTF8Seq& ch) const
{
    if (ch.is_valid())
    {
        return trim_end(U8StringView{ &ch.data[0], ch.len });
    }
    else
    {
        return { *this };
    }
}

// trim invalid
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_invalid() const
{
    return trim_invalid_start().trim_invalid_end();
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_invalid_start() const
{
    if (is_empty())
    {
        return {};
    }

    for (auto cursor = cursor_begin(); !cursor.reach_end(); cursor.move_next())
    {
        if (cursor.ref().is_valid())
        {
            return this->subview(cursor.index());
        }
    }
    return {};
}
template <typename TS>
inline constexpr U8StringView<TS> U8StringView<TS>::trim_invalid_end() const
{
    if (is_empty())
    {
        return {};
    }

    for (auto cursor = cursor_end(); !cursor.reach_begin(); cursor.move_prev())
    {
        if (cursor.ref().is_valid())
        {
            return this->subview(0, cursor.index() + cursor.seq_len());
        }
    }
    return {};
}

// partition
template <typename TS>
inline constexpr U8StringView<TS>::PartitionResult U8StringView<TS>::partition(const U8StringView& delimiter) const
{
    if (auto found = find(delimiter))
    {
        return {
            subview(0, found.index()),
            subview(found.index(), delimiter.size()),
            subview(found.index() + delimiter.size())
        };
    }
    else
    {
        return {
            *this,
            {},
            {}
        };
    }
}

// split
template <typename TS>
template <CanAdd<const U8StringView<TS>&> Buffer>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::split(Buffer& out, const U8StringView& delimiter, bool cull_empty, SizeType limit) const
{
    U8StringView each_view{ *this };
    SizeType     count = 0;
    while (true)
    {
        const auto [left, mid, right] = each_view.partition(delimiter);

        // append
        if (!cull_empty || !left.is_empty())
        {
            ++count;
            if constexpr (HasAdd<Buffer, U8StringView>)
            {
                out.add(left);
            }
            else if constexpr (HasAppend<Buffer, U8StringView<TS>>)
            {
                out.append(left);
            }
            else
            {
                static_assert(HasAdd<Buffer, U8StringView>, "Buffer must have add or append method");
            }
        }

        // trigger end
        if (right.is_empty())
        {
            break;
        }

        // limit break
        if (limit != npos && count >= limit)
        {
            break;
        }

        // update
        each_view = right;
    }
    return count;
}
template <typename TS>
template <std::invocable<const U8StringView<TS>&> F>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::split_each(F&& func, const U8StringView& delimiter, bool cull_empty, SizeType limit) const
{
    using EachFuncResultType = std::invoke_result_t<F, const U8StringView<TS>&>;

    U8StringView each_view{ *this };
    SizeType     count = 0;
    while (true)
    {
        const auto [left, mid, right] = each_view.partition(delimiter);

        // append
        if (!cull_empty || !left.is_empty())
        {
            ++count;
            if constexpr (std::is_same_v<EachFuncResultType, bool>)
            {
                if (!func(left))
                {
                    break;
                }
            }
            else
            {
                func(left);
            }
        }

        // trigger end
        if (right.is_empty())
        {
            break;
        }

        // limit break
        if (limit != npos && count >= limit)
        {
            break;
        }

        // update
        each_view = right;
    }
    return count;
}
template <typename TS>
template <CanAdd<const U8StringView<TS>&> Buffer>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::split(Buffer& out, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return split(
    out,
    delimiter.is_valid() ? U8StringView{ &delimiter.data[0], delimiter.len } : U8StringView{},
    cull_empty,
    limit);
}
template <typename TS>
template <std::invocable<const U8StringView<TS>&> F>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::split_each(F&& func, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return split_each(
    std::forward<F>(func),
    delimiter.is_valid() ? U8StringView{ &delimiter.data[0], delimiter.len } : U8StringView{},
    cull_empty,
    limit);
}

// text index
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::buffer_index_to_text(SizeType index) const
{
    return utf8_code_point_index(_data, _size, index);
}
template <typename TS>
inline constexpr U8StringView<TS>::SizeType U8StringView<TS>::text_index_to_buffer(SizeType index) const
{
    return utf8_code_unit_index(_data, _size, index);
}

// cursor & iter
template <typename TS>
inline U8StringView<TS>::CCursor U8StringView<TS>::cursor_begin() const
{
    return CCursor::Begin(_data, _size);
}
template <typename TS>
inline U8StringView<TS>::CCursor U8StringView<TS>::cursor_end() const
{
    return CCursor::End(_data, _size);
}
template <typename TS>
inline U8StringView<TS>::CIter U8StringView<TS>::iter() const
{
    return CCursor::Begin(_data, _size).as_iter();
}
template <typename TS>
inline U8StringView<TS>::CIterInv U8StringView<TS>::iter_inv() const
{
    return CCursor::End(_data, _size).as_iter_inv();
}
template <typename TS>
inline U8StringView<TS>::CRange U8StringView<TS>::range() const
{
    return CCursor::Begin(_data, _size).as_range();
}
template <typename TS>
inline U8StringView<TS>::CRangeInv U8StringView<TS>::range_inv() const
{
    return CCursor::End(_data, _size).as_range_inv();
}
} // namespace skr::container

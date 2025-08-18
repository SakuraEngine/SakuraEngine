#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include <type_traits>
#include "SkrBase/containers/string/string_view.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include "SkrBase/unicode/unicode_iterator.hpp"
#include "SkrBase/containers/string/literal.hpp"
#include "SkrBase/memory.hpp"

namespace skr::container
{
// TODO. 一些字符串工具函数
//  isalnum 全是字符或者数字
//  isalpha 全是字符
//  isdigit 是否只包含数字字符
//  islower 是否只包含小写字符
//  isupper 是否只包含大写字符
// TODO. iterator
// TODO. append repeat
template <typename Memory>
struct U8String : protected Memory {
    // from memory
    using typename Memory::DataType;
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;
    using Memory::SSOBufferSize;
    using Memory::SSOCapacity;

    // data ref
    using DataRef  = StringDataRef<DataType, SizeType, false>;
    using CDataRef = StringDataRef<DataType, SizeType, true>;

    // cursor & iterator
    using Cursor    = UTF8Cursor<SizeType, false>;
    using CCursor   = UTF8Cursor<SizeType, true>;
    using Iter      = UTF8Iter<SizeType, false>;
    using CIter     = UTF8Iter<SizeType, true>;
    using IterInv   = UTF8IterInv<SizeType, false>;
    using CIterInv  = UTF8IterInv<SizeType, true>;
    using Range     = UTF8Range<SizeType, false>;
    using CRange    = UTF8Range<SizeType, true>;
    using RangeInv  = UTF8RangeInv<SizeType, true>;
    using CRangeInv = UTF8RangeInv<SizeType, true>;

    // other types
    using ViewType        = U8StringView<SizeType>;
    using PartitionResult = StringPartitionResult<ViewType>;
    template <typename T>
    using ParseResult = StringParseResult<T, ViewType>;

    // helper
    using CharTraits               = std::char_traits<DataType>;
    static constexpr SizeType npos = npos_of<SizeType>;

    // traits
    static_assert(std::is_same_v<DataType, skr_char8>, "U8String only supports char8_t");

    // ctor & dtor
    U8String(AllocatorCtorParam param = {}) noexcept;
    U8String(const DataType* str, AllocatorCtorParam param = {}) noexcept;
    U8String(const DataType* str, SizeType len, AllocatorCtorParam param = {}) noexcept;
    U8String(ViewType view, AllocatorCtorParam param = {}) noexcept;
    U8String(DataType ch, AllocatorCtorParam param = {}) noexcept;
    ~U8String();

    // cast len
    static SizeType FromLengthRaw(const char* str) noexcept;
    static SizeType FromLengthWide(const wchar_t* str) noexcept;
    static SizeType FromLengthUtf8(const skr_char8* str) noexcept;
    static SizeType FromLengthUtf16(const skr_char16* str) noexcept;
    static SizeType FromLengthUtf32(const skr_char32* str) noexcept;
    template <typename T>
    static SizeType FromLength(T* t) noexcept;

    // cast len with len
    static SizeType FromLengthRaw(const char* str, SizeType len) noexcept;
    static SizeType FromLengthWide(const wchar_t* str, SizeType len) noexcept;
    static SizeType FromLengthUtf8(const skr_char8* str, SizeType len) noexcept;
    static SizeType FromLengthUtf16(const skr_char16* str, SizeType len) noexcept;
    static SizeType FromLengthUtf32(const skr_char32* str, SizeType len) noexcept;
    template <typename T>
    static SizeType FromLength(T* t, SizeType len) noexcept;

    // factory
    static U8String FromRaw(const char* str) noexcept;
    static U8String FromWide(const wchar_t* str) noexcept;
    static U8String FromUtf8(const skr_char8* str) noexcept;
    static U8String FromUtf16(const skr_char16* str) noexcept;
    static U8String FromUtf32(const skr_char32* str) noexcept;
    template <typename T>
    static U8String From(T* t) noexcept;

    // factory with len
    static U8String FromRaw(const char* str, SizeType len) noexcept;
    static U8String FromWide(const wchar_t* str, SizeType len) noexcept;
    static U8String FromUtf8(const skr_char8* str, SizeType len) noexcept;
    static U8String FromUtf16(const skr_char16* str, SizeType len) noexcept;
    static U8String FromUtf32(const skr_char32* str, SizeType len) noexcept;
    template <typename T>
    static U8String From(T* t, SizeType len) noexcept;

    // factory ch
    static U8String FromRaw(char ch) noexcept;
    static U8String FromWide(wchar_t ch) noexcept;
    static U8String FromUtf8(skr_char8 ch) noexcept;
    static U8String FromUtf16(skr_char16 ch) noexcept;
    static U8String FromUtf32(skr_char32 ch) noexcept;
    template <typename T>
    static U8String From(T ch) noexcept;

    // join & build factory
    // TODO. join use iterator
    template <typename... Args>
    static U8String Build(Args&&... string_or_view);
    template <typename Container>
    static U8String Join(const Container& container, ViewType separator, bool skip_empty = true, ViewType trim_chs = {}) noexcept;

    // copy & move
    U8String(const U8String& other);
    U8String(U8String&& other) noexcept;

    // assign & move assign
    U8String& operator=(const U8String& rhs);
    U8String& operator=(U8String&& rhs) noexcept;

    // special assign
    void assign(const DataType* str) noexcept;
    void assign(const DataType* str, SizeType len) noexcept;
    void assign(ViewType view) noexcept;
    template <EachAbleContainer U>
    void assign(U&& container) noexcept;

    // compare
    bool operator==(const U8String& rhs) const noexcept;
    bool operator!=(const U8String& rhs) const noexcept;
    bool operator>(const U8String& rhs) const noexcept;
    bool operator<(const U8String& rhs) const noexcept;
    bool operator>=(const U8String& rhs) const noexcept;
    bool operator<=(const U8String& rhs) const noexcept;
    bool operator==(ViewType view) const noexcept;
    bool operator!=(ViewType view) const noexcept;
    bool operator>(ViewType rhs) const noexcept;
    bool operator<(ViewType rhs) const noexcept;
    bool operator>=(ViewType rhs) const noexcept;
    bool operator<=(ViewType rhs) const noexcept;
    bool operator==(const DataType* rhs) const noexcept;
    bool operator!=(const DataType* rhs) const noexcept;
    bool operator>(const DataType* rhs) const noexcept;
    bool operator<(const DataType* rhs) const noexcept;
    bool operator>=(const DataType* rhs) const noexcept;
    bool operator<=(const DataType* rhs) const noexcept;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    bool            is_empty() const;
    DataType*       data_w();
    const DataType* data() const;
    char*           data_raw_w();
    const char*     data_raw() const;
    Memory&         memory();
    const Memory&   memory() const;

    // str getter
    SizeType        length_text() const;
    SizeType        length_buffer() const;
    const DataType* u8_str() const;
    const DataType* c_str() const;
    const char*     c_str_raw() const;

    // validate
    bool is_valid_index(SizeType index) const;

    // memory op
    void clear();
    void release(SizeType reserve_capacity = 0);
    void reserve(SizeType expect_capacity);
    void shrink();
    void resize(SizeType expect_size, const DataType& new_value);
    void resize_unsafe(SizeType expect_size);
    void resize_default(SizeType expect_size);
    void resize_zeroed(SizeType expect_size);

    // add
    DataRef add(const DataType& v, SizeType n = 1);
    DataRef add_unsafe(SizeType n);
    DataRef add_default(SizeType n);
    DataRef add_zeroed(SizeType n);

    // add at
    void add_at(SizeType idx, const DataType& v, SizeType n = 1);
    void add_at_unsafe(SizeType idx, SizeType n);
    void add_at_default(SizeType idx, SizeType n);
    void add_at_zeroed(SizeType idx, SizeType n);

    // append
    DataRef append(const DataType* str);
    DataRef append(const DataType* str, SizeType len);
    DataRef append(ViewType view);
    DataRef append(UTF8Seq seq);
    template <EachAbleContainer U>
    DataRef append(const U& container);

    // append at
    void append_at(SizeType idx, const DataType* str);
    void append_at(SizeType idx, const DataType* str, SizeType len);
    void append_at(SizeType idx, ViewType view);
    void append_at(SizeType idx, UTF8Seq seq);
    template <EachAbleContainer U>
    void append_at(SizeType idx, const U& container);

    // remove
    void     remove_at(SizeType index, SizeType n = 1);
    bool     remove(ViewType view);
    bool     remove_last(ViewType view);
    SizeType remove_all(ViewType view);
    bool     remove(UTF8Seq seq);
    bool     remove_last(UTF8Seq seq);
    SizeType remove_all(UTF8Seq seq);
    U8String remove_copy(ViewType view) const;
    U8String remove_last_copy(ViewType view) const;
    U8String remove_all_copy(ViewType view) const;
    U8String remove_copy(UTF8Seq seq) const;
    U8String remove_last_copy(UTF8Seq seq) const;
    U8String remove_all_copy(UTF8Seq seq) const;

    // replace
    SizeType replace(ViewType from, ViewType to, SizeType start = 0, SizeType count = npos);
    U8String replace_copy(ViewType from, ViewType to, SizeType start = 0, SizeType count = npos) const;
    void     replace_range(ViewType to, SizeType start = 0, SizeType count = npos);
    U8String replace_range_copy(ViewType to, SizeType start = 0, SizeType count = npos) const;

    // index & modify
    const DataType& at_buffer(SizeType index) const;
    const DataType& last_buffer(SizeType index) const;
    DataType&       at_buffer_w(SizeType index);
    DataType&       last_buffer_w(SizeType index);
    UTF8Seq         at_text(SizeType index) const;
    UTF8Seq         last_text(SizeType index) const;
    //! NOTE. if you want to modify seq (code point), use replace or remove

    // sub string
    void     first(SizeType count);
    void     last(SizeType count);
    void     substr(SizeType start, SizeType count = npos);
    U8String first_copy(SizeType count) const;
    U8String last_copy(SizeType count) const;
    U8String substr_copy(SizeType start, SizeType count = npos) const;

    // sub view
    ViewType first_view(SizeType count) const;
    ViewType last_view(SizeType count) const;
    ViewType subview(SizeType start, SizeType count = npos) const;

    // find
    CDataRef find(const ViewType& pattern) const;
    CDataRef find_last(const ViewType& pattern) const;
    CDataRef find(const UTF8Seq& pattern) const;
    CDataRef find_last(const UTF8Seq& pattern) const;
    DataRef  find_w(const ViewType& pattern);
    DataRef  find_last_w(const ViewType& pattern);
    DataRef  find_w(const UTF8Seq& pattern);
    DataRef  find_last_w(const UTF8Seq& pattern);

    // contains & count
    bool     contains(const ViewType& pattern) const;
    SizeType count(const ViewType& pattern) const;
    bool     contains(const UTF8Seq& pattern) const;
    SizeType count(const UTF8Seq& pattern) const;

    // starts & ends
    bool starts_with(const ViewType& prefix) const;
    bool ends_with(const ViewType& suffix) const;
    bool starts_with(const UTF8Seq& prefix) const;
    bool ends_with(const UTF8Seq& suffix) const;

    // remove prefix & suffix
    bool     remove_prefix(const ViewType& prefix);
    bool     remove_suffix(const ViewType& suffix);
    bool     remove_prefix(const UTF8Seq& prefix);
    bool     remove_suffix(const UTF8Seq& suffix);
    U8String remove_prefix_copy(const ViewType& prefix) const;
    U8String remove_suffix_copy(const ViewType& suffix) const;
    U8String remove_prefix_copy(const UTF8Seq& prefix) const;
    U8String remove_suffix_copy(const UTF8Seq& suffix) const;

    // trim
    bool     trim(const ViewType& characters = u8" \t");
    bool     trim_start(const ViewType& characters = u8" \t");
    bool     trim_end(const ViewType& characters = u8" \t");
    bool     trim(const UTF8Seq& ch);
    bool     trim_start(const UTF8Seq& ch);
    bool     trim_end(const UTF8Seq& ch);
    U8String trim_copy(const ViewType& characters = u8" \t") const;
    U8String trim_start_copy(const ViewType& characters = u8" \t") const;
    U8String trim_end_copy(const ViewType& characters = u8" \t") const;
    U8String trim_copy(const UTF8Seq& ch) const;
    U8String trim_start_copy(const UTF8Seq& ch) const;
    U8String trim_end_copy(const UTF8Seq& ch) const;

    // trim_invalid
    bool     trim_invalid();
    bool     trim_invalid_start();
    bool     trim_invalid_end();
    U8String trim_invalid_copy() const;
    U8String trim_invalid_start_copy() const;
    U8String trim_invalid_end_copy() const;

    // partition
    PartitionResult partition(const ViewType& delimiter) const;
    PartitionResult partition(const UTF8Seq& delimiter) const;

    // split
    template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
    SizeType split(Buffer& out, const ViewType& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
    SizeType split_each(F&& func, const ViewType& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
    SizeType split(Buffer& out, const UTF8Seq& delimiter, bool cull_empty = false, SizeType limit = npos) const;
    template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
    SizeType split_each(F&& func, const UTF8Seq& delimiter, bool cull_empty = false, SizeType limit = npos) const;

    // reverse
    void reverse(SizeType start = 0, SizeType count = npos);

    // convert size
    SizeType to_raw_length() const;
    SizeType to_wide_length() const;
    SizeType to_u8_length() const;
    SizeType to_u16_length() const;
    SizeType to_u32_length() const;
    template <typename T>
    SizeType to_length() const;

    // convert
    // !NOTE: please ensure the buffer is large enough
    void to_raw(char* buffer) const;
    void to_wide(wchar_t* buffer) const;
    void to_u8(skr_char8* buffer) const;
    void to_u16(skr_char16* buffer) const;
    void to_u32(skr_char32* buffer) const;
    template <typename T>
    void to(T* buffer) const;

    // convert append
    DataRef append(const char* str);
    DataRef append(const wchar_t* str);
    DataRef append(const skr_char16* str);
    DataRef append(const skr_char32* str);
    DataRef append(const char* str, SizeType len);
    DataRef append(const wchar_t* str, SizeType len);
    DataRef append(const skr_char16* str, SizeType len);
    DataRef append(const skr_char32* str, SizeType len);

    // convert append at
    void append_at(SizeType idx, const char* str);
    void append_at(SizeType idx, const wchar_t* str);
    void append_at(SizeType idx, const skr_char16* str);
    void append_at(SizeType idx, const skr_char32* str);
    void append_at(SizeType idx, const char* str, SizeType len);
    void append_at(SizeType idx, const wchar_t* str, SizeType len);
    void append_at(SizeType idx, const skr_char16* str, SizeType len);
    void append_at(SizeType idx, const skr_char32* str, SizeType len);

    // parse
    ParseResult<int64_t>  parse_int(uint32_t base = 10) const;
    ParseResult<uint64_t> parse_uint(uint32_t base = 10) const;
    ParseResult<double>   parse_float() const;

    // text index
    SizeType buffer_index_to_text(SizeType index) const;
    SizeType text_index_to_buffer(SizeType index) const;

    // cast to view
    operator ViewType() const;

    // case convert
    void     to_lower();
    void     to_upper();
    U8String to_lower_copy() const;
    U8String to_upper_copy() const;

    // pad
    void     pad_center(SizeType size, const DataType& ch = u8' ');
    void     pad_left(SizeType size, const DataType& ch = u8' ');
    void     pad_right(SizeType size, const DataType& ch = u8' ');
    U8String pad_center_copy(SizeType size, const DataType& ch = u8' ') const;
    U8String pad_left_copy(SizeType size, const DataType& ch = u8' ') const;
    U8String pad_right_copy(SizeType size, const DataType& ch = u8' ') const;

    // cursor & iter
    Cursor   cursor_begin();
    CCursor  cursor_begin() const;
    Cursor   cursor_end();
    CCursor  cursor_end() const;
    Iter     iter();
    CIter    iter() const;
    IterInv  iter_inv();
    CIterInv iter_inv() const;
    auto     range();
    auto     range() const;
    auto     range_inv();
    auto     range_inv() const;

    // syntax
    const U8String& readonly() const;
    ViewType        view() const;
    bool            force_cancel_literal();

private:
    // algo helper
    static void _parse_from_utf16(const skr_char16* str, SizeType len, DataType* dst, SizeType except_utf8_len);
    static void _parse_from_utf32(const skr_char32* str, SizeType len, DataType* dst, SizeType except_utf8_len);
    template <typename T>
    static SizeType _length(const T* str);

    // helper
    void            _realloc(SizeType expect_capacity);
    void            _free();
    SizeType        _grow(SizeType grow_size);
    void            _set_size(SizeType new_size);
    void            _set_literal(const DataType* str, SizeType len);
    bool            _pre_modify(SizeType except_memory = 0);
    void            _assign_with_literal_check(const DataType* str, SizeType len);
    DataType*       _data();
    const DataType* _data() const;
};
} // namespace skr::container

namespace skr::container
{
// algo helper
template <typename Memory>
inline void U8String<Memory>::_parse_from_utf16(const skr_char16* str, SizeType len, DataType* dst, SizeType except_utf8_len)
{
    using Cursor = UTF16Cursor<SizeType, true>;

    SizeType write_index = 0;
    for (UTF16Seq utf16_seq : Cursor{ str, len, 0 }.as_range())
    {
        if (utf16_seq.is_valid())
        {
            UTF8Seq utf8_seq = utf16_seq;
            ::skr::memory::copy(dst + write_index, &utf8_seq.data[0], utf8_seq.len);
            write_index += utf8_seq.len;
        }
        else
        {
            ::skr::memory::copy(dst + write_index, &utf16_seq.bad_data, 1);
            ++write_index;
        }
    }

    SKR_ASSERT(write_index == except_utf8_len && "output length is not expected when utf16 to utf8");
}
template <typename Memory>
inline void U8String<Memory>::_parse_from_utf32(const skr_char32* str, SizeType len, DataType* dst, SizeType except_utf8_len)
{
    SizeType write_index = 0;
    for (SizeType i = 0; i < len; ++i)
    {
        UTF8Seq utf8_seq = str[i];
        ::skr::memory::copy(dst + write_index, &utf8_seq.data[0], utf8_seq.len);
        write_index += utf8_seq.len;
    }

    SKR_ASSERT(write_index == except_utf8_len && "output length is not expected when utf32 to utf8");
}
template <typename Memory>
template <typename T>
inline typename U8String<Memory>::SizeType U8String<Memory>::_length(const T* str)
{
    return str ? std::char_traits<T>::length(str) :
                 0;
}

// helper
template <typename Memory>
inline void U8String<Memory>::_realloc(SizeType expect_capacity)
{
    Memory::realloc(expect_capacity);
}
template <typename Memory>
inline void U8String<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::_grow(SizeType grow_size)
{
    return Memory::grow(grow_size);
}
template <typename Memory>
inline void U8String<Memory>::_set_size(SizeType new_size)
{
    Memory::set_size(new_size);
}
template <typename Memory>
inline void U8String<Memory>::_set_literal(const DataType* str, SizeType len)
{
    Memory::set_literal(str, len);
}
template <typename Memory>
inline bool U8String<Memory>::_pre_modify(SizeType except_memory)
{
    return Memory::pre_modify(except_memory);
}
template <typename Memory>
inline void U8String<Memory>::_assign_with_literal_check(const DataType* str, SizeType len)
{
    if (len)
    {
        if (in_const_segment(str) && str[len] == 0) // FIXME. if not care error result of c_str(), we can remove this check
        {
            _set_literal(str, len);
        }
        else
        {
            resize_unsafe(len);
            ::skr::memory::copy(_data(), str, len);
        }
    }
}
template <typename Memory>
inline typename U8String<Memory>::DataType* U8String<Memory>::_data()
{
    return Memory::data();
}
template <typename Memory>
inline const typename U8String<Memory>::DataType* U8String<Memory>::_data() const
{
    return Memory::data();
}

// ctor & dtor
template <typename Memory>
inline U8String<Memory>::U8String(AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
}
template <typename Memory>
inline U8String<Memory>::U8String(const DataType* str, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    _assign_with_literal_check(str, _length(str));
}
template <typename Memory>
inline U8String<Memory>::U8String(const DataType* str, SizeType len, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    _assign_with_literal_check(str, len);
}
template <typename Memory>
inline U8String<Memory>::U8String(ViewType view, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    _assign_with_literal_check(view.data(), view.size());
}
template <typename Memory>
inline U8String<Memory>::U8String(DataType ch, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    if (ch != 0)
    {
        _assign_with_literal_check(&ch, 1);
    }
}
template <typename Memory>
inline U8String<Memory>::~U8String()
{
    // handled in memory
}

// cast len
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthRaw(const char* str) noexcept
{
    return FromLengthRaw(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthWide(const wchar_t* str) noexcept
{
    return FromLengthWide(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf8(const skr_char8* str) noexcept
{
    return FromLengthUtf8(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf16(const skr_char16* str) noexcept
{
    return FromLengthUtf16(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf32(const skr_char32* str) noexcept
{
    return FromLengthUtf32(str, _length(str));
}
template <typename Memory>
template <typename T>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLength(T* t) noexcept
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, char>)
    {
        return FromLengthRaw(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, wchar_t>)
    {
        return FromLengthWide(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char8>)
    {
        return FromLengthUtf8(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char16>)
    {
        return FromLengthUtf16(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char32>)
    {
        return FromLengthUtf32(t);
    }
    else
    {
        static_assert(std::is_same_v<T, skr_char8>, "Unsupported type");
    }
}

// cast len with len
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthRaw(const char* str, SizeType len) noexcept
{
    return len;
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthWide(const wchar_t* str, SizeType len) noexcept
{
#if _WIN64
    return FromLengthUtf16(reinterpret_cast<const skr_char16*>(str), len);
#elif __linux__ || __MACH__
    return FromLengthUtf32(reinterpret_cast<const skr_char32*>(str), len);
#endif
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf8(const skr_char8* str, SizeType len) noexcept
{
    return len;
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf16(const skr_char16* str, SizeType len) noexcept
{
    using Cursor = UTF16Cursor<SizeType, true>;

    // parse utf8 str len
    SizeType utf8_len = 0;
    for (UTF16Seq utf16_seq : Cursor{ str, len, 0 }.as_range())
    {
        if (utf16_seq.is_valid())
        {
            utf8_len += utf16_seq.to_utf8_len();
        }
        else
        {
            utf8_len += 1;
        }
    }
    return utf8_len;
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLengthUtf32(const skr_char32* str, SizeType len) noexcept
{
    SizeType utf8_len = 0;
    for (SizeType i = 0; i < len; ++i)
    {
        utf8_len += utf8_seq_len(str[i]);
    }
    return utf8_len;
}
template <typename Memory>
template <typename T>
inline typename U8String<Memory>::SizeType U8String<Memory>::FromLength(T* t, SizeType len) noexcept
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, char>)
    {
        return FromLengthRaw(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, wchar_t>)
    {
        return FromLengthWide(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char8>)
    {
        return FromLengthUtf8(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char16>)
    {
        return FromLengthUtf16(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char32>)
    {
        return FromLengthUtf32(t, len);
    }
    else
    {
        static_assert(std::is_same_v<T, skr_char8>, "Unsupported type");
    }
}

// factory
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromRaw(const char* str) noexcept
{
    return FromRaw(str, _length(str));
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromWide(const wchar_t* str) noexcept
{
    return FromWide(str, _length(str));
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf8(const skr_char8* str) noexcept
{
    return FromUtf8(str, _length(str));
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf16(const skr_char16* str) noexcept
{
    return FromUtf16(str, _length(str));
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf32(const skr_char32* str) noexcept
{
    return FromUtf32(str, _length(str));
}
template <typename Memory>
template <typename T>
inline U8String<Memory> U8String<Memory>::From(T* t) noexcept
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char8>)
    {
        return FromUtf8(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char16>)
    {
        return FromUtf16(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char32>)
    {
        return FromUtf32(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, wchar_t>)
    {
        return FromWide(t);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, char>)
    {
        return FromRaw(t);
    }
    else
    {
        static_assert(std::is_same_v<T, skr_char8>, "Unsupported type");
    }
}

// factory with len
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromRaw(const char* str, SizeType len) noexcept
{
    return { reinterpret_cast<const DataType*>(str), len };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromWide(const wchar_t* str, SizeType len) noexcept
{
#if _WIN64
    return FromUtf16(reinterpret_cast<const skr_char16*>(str), len);
#elif __linux__ || __MACH__
    return FromUtf32(reinterpret_cast<const skr_char32*>(str), len);
#endif
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf8(const skr_char8* str, SizeType len) noexcept
{
    return { str, len };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf16(const skr_char16* str, SizeType len) noexcept
{
    if (len)
    {
        // parse utf8 str len
        SizeType utf8_len = FromLength(str, len);

        // combine result
        U8String result;
        result.resize_unsafe(utf8_len);
        _parse_from_utf16(str, len, result.data_w(), utf8_len);

        return result;
    }
    else
    {
        return {};
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf32(const skr_char32* str, SizeType len) noexcept
{
    if (len)
    {
        // parse utf8 str len
        SizeType utf8_len = FromLength(str, len);

        // combine result
        U8String result;
        result.resize_unsafe(utf8_len);
        _parse_from_utf32(str, len, result.data_w(), utf8_len);

        return result;
    }
    else
    {
        return {};
    }
}
template <typename Memory>
template <typename T>
inline U8String<Memory> U8String<Memory>::From(T* t, SizeType len) noexcept
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char8>)
    {
        return FromUtf8(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char16>)
    {
        return FromUtf16(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char32>)
    {
        return FromUtf32(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, wchar_t>)
    {
        return FromWide(t, len);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, char>)
    {
        return FromRaw(t, len);
    }
    else
    {
        static_assert(std::is_same_v<T, skr_char8>, "Unsupported type");
    }
}

// factory ch
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromRaw(char ch) noexcept
{
    return { reinterpret_cast<const DataType*>(&ch), 1 };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromWide(wchar_t ch) noexcept
{
#if _WIN64
    return FromUtf16(reinterpret_cast<const skr_char16*>(&ch), 1);
#elif __linux__ || __MACH__
    return FromUtf32(reinterpret_cast<const skr_char32*>(&ch), 1);
#endif
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf8(skr_char8 ch) noexcept
{
    return { &ch, 1 };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf16(skr_char16 ch) noexcept
{
    // parse utf8 str len
    SizeType utf8_len = FromLength(&ch, 1);

    // combine result
    U8String result;
    result.resize_unsafe(utf8_len);
    _parse_from_utf16(&ch, 1, result.data_w(), utf8_len);

    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::FromUtf32(skr_char32 ch) noexcept
{
    // parse utf8 str len
    SizeType utf8_len = FromLength(&ch, 1);

    // combine result
    U8String result;
    result.resize_unsafe(utf8_len);
    _parse_from_utf32(&ch, 1, result.data_w(), utf8_len);

    return result;
}
template <typename Memory>
template <typename T>
inline U8String<Memory> U8String<Memory>::From(T ch) noexcept
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char8>)
    {
        return FromUtf8(ch);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char16>)
    {
        return FromUtf16(ch);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, skr_char32>)
    {
        return FromUtf32(ch);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, wchar_t>)
    {
        return FromWide(ch);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<T>, char>)
    {
        return FromRaw(ch);
    }
    else
    {
        static_assert(std::is_same_v<T, skr_char8>, "Unsupported type");
    }
}

// join & build factory
template <typename Memory>
template <typename... Args>
inline U8String<Memory> U8String<Memory>::Build(Args&&... string_or_view)
{
    std::array<ViewType, sizeof...(Args)> args{ ViewType{ string_or_view }... };

    // calc size
    uint64_t total_size = 0;
    for (const auto& arg : args)
    {
        total_size += arg.size();
    }

    // combine
    U8String result;
    result.reserve(total_size);
    for (const auto& arg : args)
    {
        result.append(arg);
    }

    return result;
}
template <typename Memory>
template <typename Container>
inline U8String<Memory> U8String<Memory>::Join(const Container& container, ViewType separator, bool skip_empty, ViewType trim_chs) noexcept
{
    // calc size
    uint64_t total_size      = 0;
    bool     is_first_append = false;
    for (const auto& str : container)
    {
        ViewType view{ str };

        // trim
        if (!trim_chs.is_empty())
        {
            view = view.trim(trim_chs);
        }

        // skip empty
        if (skip_empty && view.is_empty()) continue;

        // append separator
        if (!is_first_append)
        {
            is_first_append = true;
        }
        else
        {
            total_size += separator.size();
        }

        // append item
        total_size += view.size();
    }

    // combine
    U8String result;
    result.reserve(total_size);
    is_first_append = false;
    for (const auto& str : container)
    {
        ViewType view{ str };

        // trim
        if (!trim_chs.is_empty())
        {
            view = view.trim(trim_chs);
        }

        // skip empty
        if (skip_empty && view.is_empty()) continue;

        // append separator
        if (!is_first_append)
        {
            is_first_append = true;
        }
        else
        {
            result.append(separator);
        }

        // append item
        result.append(view);
    }

    SKR_ASSERT(result.size() == total_size && "Join failed");

    return result;
}

// copy & move
template <typename Memory>
inline U8String<Memory>::U8String(const U8String& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
inline U8String<Memory>::U8String(U8String&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
inline U8String<Memory>& U8String<Memory>::operator=(const U8String& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
inline U8String<Memory>& U8String<Memory>::operator=(U8String&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// special assign
template <typename Memory>
inline void U8String<Memory>::assign(const DataType* str) noexcept
{
    clear();
    _assign_with_literal_check(str, _length(str));
}
template <typename Memory>
inline void U8String<Memory>::assign(const DataType* str, SizeType len) noexcept
{
    clear();
    _assign_with_literal_check(str, len);
}
template <typename Memory>
inline void U8String<Memory>::assign(ViewType view) noexcept
{
    clear();
    _assign_with_literal_check(view.data(), view.size());
}
template <typename Memory>
template <EachAbleContainer U>
inline void U8String<Memory>::assign(U&& container) noexcept
{
    using Traits = ContainerTraits<std::decay_t<U>>;

    clear();

    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        resize_unsafe(n);
        ::skr::memory::copy(_data(), p, n);
    }
    else if constexpr (Traits::is_iterable && Traits::has_size)
    {
        auto n     = Traits::size(std::forward<U>(container));
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        reserve(n);
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
    }
}

// compare
template <typename Memory>
inline bool U8String<Memory>::operator==(const U8String& rhs) const noexcept
{
    return view() == rhs.view();
}
template <typename Memory>
inline bool U8String<Memory>::operator!=(const U8String& rhs) const noexcept
{
    return view() != rhs.view();
};
template <typename Memory>
inline bool U8String<Memory>::operator>(const U8String& rhs) const noexcept
{
    return view() > rhs.view();
}
template <typename Memory>
inline bool U8String<Memory>::operator<(const U8String& rhs) const noexcept
{
    return view() < rhs.view();
}
template <typename Memory>
inline bool U8String<Memory>::operator>=(const U8String& rhs) const noexcept
{
    return view() >= rhs.view();
}
template <typename Memory>
inline bool U8String<Memory>::operator<=(const U8String& rhs) const noexcept
{
    return view() <= rhs.view();
}
template <typename Memory>
inline bool U8String<Memory>::operator==(ViewType rhs) const noexcept
{
    return view() == rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator!=(ViewType rhs) const noexcept
{
    return view() != rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator>(ViewType rhs) const noexcept
{
    return view() > rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator<(ViewType rhs) const noexcept
{
    return view() < rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator>=(ViewType rhs) const noexcept
{
    return view() >= rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator<=(ViewType rhs) const noexcept
{
    return view() <= rhs;
}
template <typename Memory>
inline bool U8String<Memory>::operator==(const DataType* rhs) const noexcept
{
    return view() == ViewType{ rhs };
}
template <typename Memory>
inline bool U8String<Memory>::operator!=(const DataType* rhs) const noexcept
{
    return view() != ViewType{ rhs };
}
template <typename Memory>
inline bool U8String<Memory>::operator>(const DataType* rhs) const noexcept
{
    return view() > ViewType{ rhs };
}
template <typename Memory>
inline bool U8String<Memory>::operator<(const DataType* rhs) const noexcept
{
    return view() < ViewType{ rhs };
}
template <typename Memory>
inline bool U8String<Memory>::operator>=(const DataType* rhs) const noexcept
{
    return view() >= ViewType{ rhs };
}
template <typename Memory>
inline bool U8String<Memory>::operator<=(const DataType* rhs) const noexcept
{
    return view() <= ViewType{ rhs };
}

// getter
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::slack() const
{
    return capacity() - size();
}
template <typename Memory>
inline bool U8String<Memory>::is_empty() const
{
    return size() == 0;
}
template <typename Memory>
inline typename U8String<Memory>::DataType* U8String<Memory>::data_w()
{
    _pre_modify();
    return _data();
}
template <typename Memory>
inline const typename U8String<Memory>::DataType* U8String<Memory>::data() const
{
    return _data();
}
template <typename Memory>
inline char* U8String<Memory>::data_raw_w()
{
    _pre_modify();
    return (char*)_data();
}
template <typename Memory>
inline const char* U8String<Memory>::data_raw() const
{
    return (const char*)_data();
}
template <typename Memory>
inline Memory& U8String<Memory>::memory()
{
    return *this;
}
template <typename Memory>
inline const Memory& U8String<Memory>::memory() const
{
    return *this;
}

// str getter
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::length_text() const
{
    return view().length_text();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::length_buffer() const
{
    return view().length_buffer();
}
template <typename Memory>
inline const typename U8String<Memory>::DataType* U8String<Memory>::u8_str() const
{
    return _data();
}
template <typename Memory>
inline const typename U8String<Memory>::DataType* U8String<Memory>::c_str() const
{
    return _data();
}
template <typename Memory>
inline const char* U8String<Memory>::c_str_raw() const
{
    return reinterpret_cast<const char*>(_data());
}

// validate
template <typename Memory>
inline bool U8String<Memory>::is_valid_index(SizeType index) const
{
    return index >= 0 && index < size();
}

// memory op
template <typename Memory>
inline void U8String<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
inline void U8String<Memory>::release(SizeType reserve_capacity)
{
    clear();
    if (reserve_capacity)
    {
        if (reserve_capacity != capacity())
        {
            _realloc(reserve_capacity);
        }
    }
    else
    {
        _free();
    }
}
template <typename Memory>
inline void U8String<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _pre_modify();
        _realloc(expect_capacity);
    }
}
template <typename Memory>
inline void U8String<Memory>::shrink()
{
    Memory::shrink();
}
template <typename Memory>
inline void U8String<Memory>::resize(SizeType expect_size, const DataType& new_value)
{
    // realloc memory if need
    if (!_pre_modify(expect_size) && expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        for (SizeType i = size(); i < expect_size; ++i)
        {
            new (_data() + i) DataType(new_value);
        }
    }
    else if (expect_size < size())
    {
        ::skr::memory::destruct(_data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
inline void U8String<Memory>::resize_unsafe(SizeType expect_size)
{
    // realloc memory if need
    if (!_pre_modify(expect_size) && expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // destruct item if need
    if (expect_size < size())
    {
        ::skr::memory::destruct(_data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
inline void U8String<Memory>::resize_default(SizeType expect_size)
{
    // realloc memory if need
    if (!_pre_modify(expect_size) && expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        ::skr::memory::construct(_data() + size(), expect_size - size());
    }
    else if (expect_size < size())
    {
        ::skr::memory::destruct(_data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
inline void U8String<Memory>::resize_zeroed(SizeType expect_size)
{
    // realloc memory if need
    if (!_pre_modify(expect_size) && expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        std::memset(reinterpret_cast<void*>(_data() + size()), 0, (expect_size - size()) * sizeof(DataType));
    }
    else if (expect_size < size())
    {
        ::skr::memory::destruct(_data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::add(const DataType& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index(); i < size(); ++i)
    {
        new (_data() + i) DataType(v);
    }
    return ref;
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::add_unsafe(SizeType n)
{
    if (n)
    {
        _pre_modify(size() + n);
        SizeType old_size = _grow(n);
        return { _data() + old_size, old_size };
    }
    else
    {
        return _data() ? DataRef(_data() + size(), size()) : DataRef();
    }
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    ::skr::memory::construct(ref.ptr(), n);
    return ref;
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::add_zeroed(SizeType n)
{
    DataRef ref = add_unsafe(n);
    std::memset(ref.ptr(), 0, n * sizeof(DataType));
    return ref;
}

// add at
template <typename Memory>
inline void U8String<Memory>::add_at(SizeType idx, const DataType& v, SizeType n)
{
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (_data() + idx + i) DataType(v);
    }
}
template <typename Memory>
inline void U8String<Memory>::add_at_unsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT((is_empty() && idx == 0) || is_valid_index(idx));
    auto move_n = size() - idx;
    add_unsafe(n);
    ::skr::memory::move(_data() + idx + n, _data() + idx, move_n);
}
template <typename Memory>
inline void U8String<Memory>::add_at_default(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    ::skr::memory::construct(_data() + idx, n);
}
template <typename Memory>
inline void U8String<Memory>::add_at_zeroed(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    std::memset(_data() + idx, 0, n * sizeof(DataType));
}

// append
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const DataType* str)
{
    return append(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const DataType* str, SizeType len)
{
    if (len)
    {
        DataRef ref = add_unsafe(len);
        ::skr::memory::copy(ref.ptr(), str, len);
        return ref;
    }
    return {};
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(ViewType view)
{
    return append(view.data(), view.size());
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return append(&seq.data[0], seq.len);
    }
    return _data() ? DataRef(_data() + size(), size()) : DataRef();
}
template <typename Memory>
template <EachAbleContainer U>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const U& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        if (n)
        {
            DataRef ref = add_unsafe(n);
            ::skr::memory::copy(ref.ptr(), p, n);
            return ref;
        }
    }
    else if constexpr (Traits::is_iterable && Traits::has_size)
    {
        auto n        = Traits::size(std::forward<U>(container));
        auto begin    = Traits::begin(std::forward<U>(container));
        auto end      = Traits::end(std::forward<U>(container));
        auto old_size = size();
        reserve(size() + n);
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
        return { _data() + old_size, old_size };
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin    = Traits::begin(std::forward<U>(container));
        auto end      = Traits::end(std::forward<U>(container));
        auto old_size = size();
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
        return { _data() + old_size, old_size };
    }
    return {};
}

// append at
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const DataType* str)
{
    if (str)
    {
        append_at(idx, str, _length(str));
    }
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const DataType* str, SizeType len)
{
    if (len)
    {
        add_at_unsafe(idx, len);
        ::skr::memory::copy(_data() + idx, str, len);
    }
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, ViewType view)
{
    append_at(idx, view.data(), view.size());
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, UTF8Seq seq)
{
    if (seq.is_valid())
    {
        append_at(idx, &seq.data[0], seq.len);
    }
}
template <typename Memory>
template <EachAbleContainer U>
inline void U8String<Memory>::append_at(SizeType idx, const U& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        if (n)
        {
            add_at_unsafe(idx, n);
            ::skr::memory::copy(_data() + idx, p, n);
        }
    }
    else if constexpr (Traits::is_iterable && Traits::has_size)
    {
        auto n        = Traits::size(std::forward<U>(container));
        auto begin    = Traits::begin(std::forward<U>(container));
        auto end      = Traits::end(std::forward<U>(container));
        auto old_size = size();
        add_at_unsafe(idx, n);
        for (; begin != end; ++begin)
        {
            new (_data() + idx) DataType(*begin);
            ++idx;
        }
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin    = Traits::begin(std::forward<U>(container));
        auto end      = Traits::end(std::forward<U>(container));
        auto old_size = size();
        for (; begin != end; ++begin)
        {
            add_at(idx, *begin);
            ++idx;
        }
    }
}

// remove
template <typename Memory>
inline void U8String<Memory>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index) && size() - index >= n);

    if (n)
    {
        _pre_modify();

        // calc move size
        auto move_n = size() - index - n;

        // destruct remove items
        ::skr::memory::destruct(_data() + index, n);

        // move data
        if (move_n)
        {
            ::skr::memory::move(_data() + index, _data() + size() - move_n, move_n);
        }

        // update size
        _set_size(size() - n);
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove(ViewType view)
{
    if (DataRef ref = find(view))
    {
        remove_at(ref.index(), view.size());
        return true;
    }
    return false;
}
template <typename Memory>
inline bool U8String<Memory>::remove_last(ViewType view)
{
    if (DataRef ref = find_last(view))
    {
        remove_at(ref.index(), view.size());
        return true;
    }
    return false;
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::remove_all(ViewType view)
{
    if (!is_empty())
    {
        CDataRef first_found = find(view);

        if (first_found)
        {
            // exit literal state
            if (_pre_modify())
            {
                first_found = { _data() + first_found.index(), first_found.index() };
            }

            DataType* write     = _data();
            DataType* read      = _data();
            SizeType  read_size = size();

            SizeType count = 0;
            DataRef  found = first_found;
            while (found)
            {
                SizeType move_count = found.ptr() - read;

                // copy data before found
                if (write != read)
                {
                    ::skr::memory::move(write, read, move_count);
                }

                // update read info
                read_size -= move_count + view.size();
                read = found.ptr() + view.size();

                // update write info
                write += move_count;

                // find next
                found = ViewType{ read, read_size }.find(view);

                ++count;
            }

            // copy final data
            ::skr::memory::move(write, read, read_size);

            // set size
            _set_size(size() - view.size() * count);

            return count;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove(UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return remove(ViewType{ &seq.data[0], seq.len });
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove_last(UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return remove_last(ViewType{ &seq.data[0], seq.len });
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::remove_all(UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return remove_all(ViewType{ &seq.data[0], seq.len });
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_copy(ViewType view) const
{
    U8String result = *this;
    result.remove(view);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_last_copy(ViewType view) const
{
    U8String result = *this;
    result.remove_last(view);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_all_copy(ViewType view) const
{
    U8String result = *this;
    result.remove_all(view);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove(seq);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_last_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove_last(seq);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_all_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove_all(seq);
    return std::move(result);
}

// replace
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::replace(ViewType from, ViewType to, SizeType start, SizeType count)
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");

    if (from.is_empty())
    {
        return 0;
    }

    // count result
    ViewType find_view     = subview(start, count);
    SizeType replace_count = find_view.count(from);
    if (replace_count == 0)
    {
        return 0;
    }
    SizeType replace_size_from = from.size() * replace_count;
    SizeType replace_size_to   = to.size() * replace_count;

    if (to.size() == from.size())
    { // just find and replace
        if (_pre_modify())
        {
            find_view = subview(start, count);
        }

        DataRef found_ref = find_view.find(from);
        while (found_ref)
        {
            // copy data
            ::skr::memory::copy(found_ref.ptr(), to.data(), to.size());

            // update view
            find_view = find_view.subview(found_ref.index() + to.size());

            // find next
            found_ref = find_view.find(from);
        }
    }
    else if (to.size() < from.size())
    { // replace from start to end
        if (_pre_modify())
        {
            find_view = subview(start, count);
        }

        DataRef   found_ref = find_view.find(from);
        DataType* read      = found_ref.ptr();
        DataType* write     = found_ref.ptr();
        while (found_ref)
        {
            // move data before found
            if (write != read)
            {
                auto move_size = found_ref.ptr() - read;
                ::skr::memory::move(write, read, move_size);
                write += move_size;
            }

            // copy replace part
            ::skr::memory::copy(write, to.data(), to.size());
            write += to.size();

            // update read and find view
            find_view = find_view.subview(found_ref.index() + from.size());

            // update read
            read = const_cast<DataType*>(find_view.data());

            // find next
            found_ref = find_view.find(from);
        }

        // move final part
        DataType* end = _data() + size();
        if (read != end)
        {
            ::skr::memory::move(write, read, end - read);
        }

        // update size
        _set_size(size() - replace_size_from + replace_size_to);
    }
    else
    { // make a new string and copy
        U8String result = replace_copy(from, to, start, count);
        *this           = std::move(result);
    }

    return replace_count;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::replace_copy(ViewType from, ViewType to, SizeType start, SizeType count) const
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");

    if (from.is_empty())
    {
        return {};
    }

    // count result
    ViewType find_view     = subview(start, count);
    SizeType replace_count = find_view.count(from);
    if (replace_count == 0)
    {
        return *this;
    }
    SizeType replace_size_from = from.size() * replace_count;
    SizeType replace_size_to   = to.size() * replace_count;

    if (to.size() <= from.size())
    {
        U8String result = *this;
        result.replace(from, to, start, count);
        return result;
    }
    else
    {
        U8String result;
        result.reserve((count == npos ? size() : count) + replace_size_to - replace_size_from);

        CDataRef        found_ref = find_view.find(from);
        const DataType* read      = _data();

        while (found_ref)
        {
            // copy data before found
            result.append(ViewType{ read, static_cast<SizeType>(found_ref.ptr() - read) });

            // copy replace part
            result.append(ViewType{ to.data(), to.size() });

            // update read and find view
            find_view = find_view.subview(found_ref.index() + from.size());
            read      = find_view.data();

            // find next
            found_ref = find_view.find(from);
        }

        // copy final part
        result.append(ViewType{ read, static_cast<SizeType>(_data() + size() - read) });
        return result;
    }
}
template <typename Memory>
inline void U8String<Memory>::replace_range(ViewType to, SizeType start, SizeType count)
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");

    count = count == npos ? size() - start : count;

    if (count == 0)
    { // no replace
        return;
    }
    if (start == 0 && count == size())
    { // full replace
        clear();
        append(to);
        return;
    }

    if (to.size() <= count)
    { // no need to reserve
        _pre_modify();

        // copy to
        ::skr::memory::copy(_data() + start, to.data(), to.size());

        // move rest
        ::skr::memory::move(_data() + start + to.size(), _data() + start + count, size() - start - count);
    }
    else
    { // need to reserve
        SizeType reserve_size = size() + to.size() - count;
        _pre_modify(reserve_size);
        reserve(reserve_size);

        // move rest
        ::skr::memory::move(_data() + start + to.size(), _data() + start + count, size() - start - count);

        // copy to
        ::skr::memory::copy(_data() + start, to.data(), to.size());
    }

    // update size
    _set_size(size() - count + to.size());
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::replace_range_copy(ViewType to, SizeType start, SizeType count) const
{
    U8String result = *this;
    result.replace_range(to, start, count);
    return std::move(result);
}

// index & modify
template <typename Memory>
inline const typename U8String<Memory>::DataType& U8String<Memory>::at_buffer(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    return _data()[index];
}
template <typename Memory>
inline const typename U8String<Memory>::DataType& U8String<Memory>::last_buffer(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    return _data()[size() - index - 1];
}
template <typename Memory>
inline typename U8String<Memory>::DataType& U8String<Memory>::at_buffer_w(SizeType index)
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    _pre_modify();
    return _data()[index];
}
template <typename Memory>
inline typename U8String<Memory>::DataType& U8String<Memory>::last_buffer_w(SizeType index)
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    _pre_modify();
    return _data()[size() - index - 1];
}
template <typename Memory>
inline UTF8Seq U8String<Memory>::at_text(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    uint64_t seq_index;
    return UTF8Seq::ParseUTF8(_data(), size(), index, seq_index);
}
template <typename Memory>
inline UTF8Seq U8String<Memory>::last_text(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    uint64_t seq_index;
    return UTF8Seq::ParseUTF8(_data(), size(), size() - index - 1, seq_index);
}

// sub_string
template <typename Memory>
inline void U8String<Memory>::first(SizeType count)
{
    SKR_ASSERT(count <= size() && "undefined behaviour exceeding size of string view");
    if (count == size())
    {
        return;
    }
    remove_at(count, size() - count);
}
template <typename Memory>
inline void U8String<Memory>::last(SizeType count)
{
    SKR_ASSERT(count <= size() && "undefined behaviour exceeding size of string view");
    if (count == size())
    {
        return;
    }
    remove_at(0, size() - count);
}
template <typename Memory>
inline void U8String<Memory>::substr(SizeType start, SizeType count)
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");
    count = count == npos ? size() - start : count;

    if (start == 0 && count == size())
    { // clear case
        clear();
        return;
    }
    else
    { // move center
        _pre_modify();
        ::skr::memory::move(_data(), _data() + start, count);
        _set_size(count);
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::first_copy(SizeType count) const
{
    SKR_ASSERT(count <= size() && "undefined behaviour exceeding size of string view");
    if (count == size())
    {
        return *this;
    }
    else
    {
        return { view().first(count) };
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::last_copy(SizeType count) const
{
    SKR_ASSERT(count <= size() && "undefined behaviour exceeding size of string view");
    if (count == size())
    {
        return *this;
    }
    else
    {
        return { view().last(count) };
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::substr_copy(SizeType start, SizeType count) const
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");
    if (start == 0 && count == size())
    {
        return *this;
    }
    else
    {
        return { view().subview(start, count) };
    }
}

// sub view
template <typename Memory>
inline typename U8String<Memory>::ViewType U8String<Memory>::first_view(SizeType count) const
{
    return view().first(count);
}
template <typename Memory>
inline typename U8String<Memory>::ViewType U8String<Memory>::last_view(SizeType count) const
{
    return view().last(count);
}
template <typename Memory>
inline typename U8String<Memory>::ViewType U8String<Memory>::subview(SizeType start, SizeType count) const
{
    return view().subview(start, count);
}

// find
template <typename Memory>
inline typename U8String<Memory>::CDataRef U8String<Memory>::find(const ViewType& pattern) const
{
    return view().find(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::CDataRef U8String<Memory>::find_last(const ViewType& pattern) const
{
    return view().find_last(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::CDataRef U8String<Memory>::find(const UTF8Seq& pattern) const
{
    return view().find(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::CDataRef U8String<Memory>::find_last(const UTF8Seq& pattern) const
{
    return view().find_last(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::find_w(const ViewType& pattern)
{
    _pre_modify();
    return view().find(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::find_last_w(const ViewType& pattern)
{
    _pre_modify();
    return view().find_last(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::find_w(const UTF8Seq& pattern)
{
    _pre_modify();
    return view().find(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::find_last_w(const UTF8Seq& pattern)
{
    _pre_modify();
    return view().find_last(pattern);
}

// contains & count
template <typename Memory>
inline bool U8String<Memory>::contains(const ViewType& pattern) const
{
    return view().contains(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::count(const ViewType& pattern) const
{
    return view().count(pattern);
}
template <typename Memory>
inline bool U8String<Memory>::contains(const UTF8Seq& pattern) const
{
    return view().contains(pattern);
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::count(const UTF8Seq& pattern) const
{
    return view().count(pattern);
}

// starts & ends
template <typename Memory>
inline bool U8String<Memory>::starts_with(const ViewType& prefix) const
{
    return view().starts_with(prefix);
}
template <typename Memory>
inline bool U8String<Memory>::ends_with(const ViewType& suffix) const
{
    return view().ends_with(suffix);
}
template <typename Memory>
inline bool U8String<Memory>::starts_with(const UTF8Seq& prefix) const
{
    return view().starts_with(prefix);
}
template <typename Memory>
inline bool U8String<Memory>::ends_with(const UTF8Seq& suffix) const
{
    return view().ends_with(suffix);
}

// remove prefix & suffix
template <typename Memory>
inline bool U8String<Memory>::remove_prefix(const ViewType& prefix)
{
    if (starts_with(prefix))
    {
        remove_at(0, prefix.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove_suffix(const ViewType& suffix)
{
    if (ends_with(suffix))
    {
        remove_at(size() - suffix.size(), suffix.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove_prefix(const UTF8Seq& prefix)
{
    if (starts_with(prefix))
    {
        remove_at(0, prefix.len);
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::remove_suffix(const UTF8Seq& suffix)
{
    if (ends_with(suffix))
    {
        remove_at(size() - suffix.len, suffix.len);
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_prefix_copy(const ViewType& prefix) const
{
    return { view().remove_prefix(prefix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_suffix_copy(const ViewType& suffix) const
{
    return { view().remove_suffix(suffix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_prefix_copy(const UTF8Seq& prefix) const
{
    return { view().remove_prefix(prefix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_suffix_copy(const UTF8Seq& suffix) const
{
    return { view().remove_suffix(suffix) };
}

// trim
template <typename Memory>
inline bool U8String<Memory>::trim(const ViewType& characters)
{
    bool any_trim = false;
    any_trim |= trim_start(characters);
    any_trim |= trim_end(characters);
    return any_trim;
}
template <typename Memory>
inline bool U8String<Memory>::trim_start(const ViewType& characters)
{
    ViewType trim_view = view().trim_start(characters);
    if (trim_view.size() != size())
    {
        _pre_modify();
        ::skr::memory::move(_data(), const_cast<DataType*>(trim_view.data()), trim_view.size());
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::trim_end(const ViewType& characters)
{
    ViewType trim_view = view().trim_end(characters);
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::trim(const UTF8Seq& ch)
{
    bool any_trim = false;
    any_trim |= trim_start(ch);
    any_trim |= trim_end(ch);
    return any_trim;
}
template <typename Memory>
inline bool U8String<Memory>::trim_start(const UTF8Seq& ch)
{
    ViewType trim_view = view().trim_start(ch);
    if (trim_view.size() != size())
    {
        _pre_modify();
        ::skr::memory::move(_data(), const_cast<DataType*>(trim_view.data()), trim_view.size());
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::trim_end(const UTF8Seq& ch)
{
    ViewType trim_view = view().trim_end(ch);
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_copy(const ViewType& characters) const
{
    return { view().trim(characters) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_start_copy(const ViewType& characters) const
{
    return { view().trim_start(characters) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_end_copy(const ViewType& characters) const
{
    return { view().trim_end(characters) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_copy(const UTF8Seq& ch) const
{
    return { view().trim(ch) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_start_copy(const UTF8Seq& ch) const
{
    return { view().trim_start(ch) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_end_copy(const UTF8Seq& ch) const
{
    return { view().trim_end(ch) };
}

// trim_invalid
template <typename Memory>
inline bool U8String<Memory>::trim_invalid()
{
    bool any_trim = false;
    any_trim |= trim_invalid_start();
    any_trim |= trim_invalid_end();
    return any_trim;
}
template <typename Memory>
inline bool U8String<Memory>::trim_invalid_start()
{
    ViewType trim_view = view().trim_invalid_start();
    if (trim_view.size() != size())
    {
        _pre_modify();
        ::skr::memory::move(_data(), const_cast<DataType*>(trim_view.data()), trim_view.size());
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline bool U8String<Memory>::trim_invalid_end()
{
    ViewType trim_view = view().trim_invalid_end();
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_invalid_copy() const
{
    return { view().trim_invalid() };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_invalid_start_copy() const
{
    return { view().trim_invalid_start() };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::trim_invalid_end_copy() const
{
    return { view().trim_invalid_end() };
}

// partition
template <typename Memory>
inline typename U8String<Memory>::PartitionResult U8String<Memory>::partition(const ViewType& delimiter) const
{
    return view().partition(delimiter);
}
template <typename Memory>
inline typename U8String<Memory>::PartitionResult U8String<Memory>::partition(const UTF8Seq& delimiter) const
{
    return view().partition(delimiter);
}

// split
template <typename Memory>
template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
inline typename U8String<Memory>::SizeType U8String<Memory>::split(Buffer& out, const ViewType& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split(out, delimiter, cull_empty, limit);
}
template <typename Memory>
template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
inline typename U8String<Memory>::SizeType U8String<Memory>::split_each(F&& func, const ViewType& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split_each(std::forward<F>(func), delimiter, cull_empty, limit);
}
template <typename Memory>
template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
inline typename U8String<Memory>::SizeType U8String<Memory>::split(Buffer& out, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split(out, delimiter, cull_empty, limit);
}
template <typename Memory>
template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
inline typename U8String<Memory>::SizeType U8String<Memory>::split_each(F&& func, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split_each(std::forward<F>(func), delimiter, cull_empty, limit);
}

// reverse
template <typename Memory>
inline void U8String<Memory>::reverse(SizeType start, SizeType count)
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");
    count = count == npos ? size() - start : count;

    if (count > 1)
    {
        _pre_modify();
        SizeType end = start + count - 1;
        DataType tmp;
        for (SizeType i = 0; i < count / 2; ++i)
        {
            tmp                = _data()[start + i];
            _data()[start + i] = _data()[end - i];
            _data()[end - i]   = tmp;
        }
    }
}

// convert size
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_raw_length() const
{
    return view().to_raw_length();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_wide_length() const
{
    return view().to_wide_length();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_u8_length() const
{
    return view().to_u8_length();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_u16_length() const
{
    return view().to_u16_length();
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_u32_length() const
{
    return view().to_u32_length();
}
template <typename Memory>
template <typename T>
inline typename U8String<Memory>::SizeType U8String<Memory>::to_length() const
{
    return view().template to_length<T>();
}

// convert
template <typename Memory>
inline void U8String<Memory>::to_raw(char* buffer) const
{
    return view().to_raw(buffer);
}
template <typename Memory>
inline void U8String<Memory>::to_wide(wchar_t* buffer) const
{
    return view().to_wide(buffer);
}
template <typename Memory>
inline void U8String<Memory>::to_u8(skr_char8* buffer) const
{
    return view().to_u8(buffer);
}
template <typename Memory>
inline void U8String<Memory>::to_u16(skr_char16* buffer) const
{
    return view().to_u16(buffer);
}
template <typename Memory>
inline void U8String<Memory>::to_u32(skr_char32* buffer) const
{
    return view().to_u32(buffer);
}
template <typename Memory>
template <typename T>
inline void U8String<Memory>::to(T* buffer) const
{
    return view().to(buffer);
}

// convert append
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const char* str)
{
    return append(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const wchar_t* str)
{
    return append(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const skr_char16* str)
{
    return append(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const skr_char32* str)
{
    return append(str, _length(str));
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const char* str, SizeType len)
{
    return append(reinterpret_cast<const DataType*>(str), len);
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const wchar_t* str, SizeType len)
{
#if _WIN64
    return append(reinterpret_cast<const skr_char16*>(str));
#elif __linux__ || __MACH__
    return append(reinterpret_cast<const skr_char32*>(str));
#endif
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const skr_char16* str, SizeType len)
{
    if (len)
    {
        SizeType utf8_len   = FromLength(str, len);
        DataRef  add_result = add_unsafe(utf8_len);
        _parse_from_utf16(str, len, add_result.ptr(), utf8_len);
        return add_result;
    }
    return {};
}
template <typename Memory>
inline typename U8String<Memory>::DataRef U8String<Memory>::append(const skr_char32* str, SizeType len)
{
    if (len)
    {
        SizeType utf8_len   = FromLength(str, len);
        DataRef  add_result = add_unsafe(utf8_len);
        _parse_from_utf32(str, len, add_result.ptr(), utf8_len);
        return add_result;
    }
    return {};
}

// convert append at
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const char* str)
{
    return append_at(idx, str, _length(str));
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const wchar_t* str)
{
    return append_at(idx, str, _length(str));
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const skr_char16* str)
{
    return append_at(idx, str, _length(str));
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const skr_char32* str)
{
    return append_at(idx, str, _length(str));
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const char* str, SizeType len)
{
    return append_at(idx, reinterpret_cast<const DataType*>(str), len);
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const wchar_t* str, SizeType len)
{
#if _WIN64
    return append(reinterpret_cast<const skr_char16*>(str));
#elif __linux__ || __MACH__
    return append(reinterpret_cast<const skr_char32*>(str));
#endif
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const skr_char16* str, SizeType len)
{
    if (len)
    {
        SizeType utf8_len = FromLength(str, len);
        add_at_unsafe(idx, utf8_len);
        _parse_from_utf16(str, len, _data() + idx, utf8_len);
    }
}
template <typename Memory>
inline void U8String<Memory>::append_at(SizeType idx, const skr_char32* str, SizeType len)
{
    if (len)
    {
        SizeType utf8_len = FromLength(str, len);
        add_at_unsafe(idx, utf8_len);
        _parse_from_utf32(str, len, _data() + idx, utf8_len);
    }
}

// parse
template <typename Memory>
inline U8String<Memory>::ParseResult<int64_t> U8String<Memory>::parse_int(uint32_t base) const
{
    return view().parse_int(base);
}
template <typename Memory>
inline U8String<Memory>::ParseResult<uint64_t> U8String<Memory>::parse_uint(uint32_t base) const
{
    return view().parse_uint(base);
}
template <typename Memory>
inline U8String<Memory>::ParseResult<double> U8String<Memory>::parse_float() const
{
    return view().parse_float();
}

// text index
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::buffer_index_to_text(SizeType index) const
{
    return view().buffer_index_to_text(index);
}
template <typename Memory>
inline typename U8String<Memory>::SizeType U8String<Memory>::text_index_to_buffer(SizeType index) const
{
    return view().text_index_to_buffer(index);
}

// cast to view
template <typename Memory>
inline U8String<Memory>::operator ViewType() const
{
    return view();
}

// case convert
template <typename Memory>
inline void U8String<Memory>::to_lower()
{
    _pre_modify();
    for (int32_t i = 0; i < length_buffer(); ++i)
    {
        DataType& ch = _data()[i];
        ch           = static_cast<DataType>(std::tolower(static_cast<int>(ch)));
    }
}
template <typename Memory>
inline void U8String<Memory>::to_upper()
{
    _pre_modify();
    for (int32_t i = 0; i < length_buffer(); ++i)
    {
        DataType& ch = _data()[i];
        ch           = static_cast<DataType>(std::toupper(static_cast<int>(ch)));
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::to_lower_copy() const
{
    U8String result{ *this };
    result.to_lower();
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::to_upper_copy() const
{
    U8String result{ *this };
    result.to_upper();
    return std::move(result);
}

// pad
template <typename Memory>
inline void U8String<Memory>::pad_center(SizeType size, const DataType& ch)
{
    if (size > length_buffer())
    {
        _pre_modify(size);

        SizeType size_need_pad = size - length_buffer();
        SizeType left          = size_need_pad / 2;
        SizeType right         = size_need_pad - left;

        if (left > 0)
        {
            add_at(0, ch, left);
        }
        if (right > 0)
        {
            add(ch, right);
        }
    }
}
template <typename Memory>
inline void U8String<Memory>::pad_left(SizeType size, const DataType& ch)
{
    if (size > length_buffer())
    {
        _pre_modify(size);
        SizeType size_need_pad = size - length_buffer();
        add_at(0, ch, size_need_pad);
    }
}
template <typename Memory>
inline void U8String<Memory>::pad_right(SizeType size, const DataType& ch)
{
    if (size > length_buffer())
    {
        _pre_modify(size);
        SizeType size_need_pad = size - length_buffer();
        add(ch, size_need_pad);
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::pad_center_copy(SizeType size, const DataType& ch) const
{
    U8String result = *this;
    result.pad_center(size, ch);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::pad_left_copy(SizeType size, const DataType& ch) const
{
    U8String result = *this;
    result.pad_left(size, ch);
    return std::move(result);
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::pad_right_copy(SizeType size, const DataType& ch) const
{
    U8String result = *this;
    result.pad_right(size, ch);
    return std::move(result);
}

// cursor & iter
template <typename Memory>
inline typename U8String<Memory>::Cursor U8String<Memory>::cursor_begin()
{
    return Cursor::Begin(_data(), length_buffer());
}
template <typename Memory>
inline typename U8String<Memory>::CCursor U8String<Memory>::cursor_begin() const
{
    return CCursor::Begin(_data(), length_buffer());
}
template <typename Memory>
inline typename U8String<Memory>::Cursor U8String<Memory>::cursor_end()
{
    return Cursor::End(_data(), length_buffer());
}
template <typename Memory>
inline typename U8String<Memory>::CCursor U8String<Memory>::cursor_end() const
{
    return CCursor::End(_data(), length_buffer());
}
template <typename Memory>
inline typename U8String<Memory>::Iter U8String<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
inline typename U8String<Memory>::CIter U8String<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
inline typename U8String<Memory>::IterInv U8String<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
inline typename U8String<Memory>::CIterInv U8String<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
inline auto U8String<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
inline auto U8String<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
inline auto U8String<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
inline auto U8String<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// syntax
template <typename Memory>
inline const U8String<Memory>& U8String<Memory>::readonly() const
{
    return *this;
}
template <typename Memory>
inline typename U8String<Memory>::ViewType U8String<Memory>::view() const
{
    return { _data(), size() };
}
template <typename Memory>
inline bool U8String<Memory>::force_cancel_literal()
{
    return _pre_modify();
}
} // namespace skr::container
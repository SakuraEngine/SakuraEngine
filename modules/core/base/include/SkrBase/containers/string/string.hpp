#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include "SkrBase/containers/string/string.hpp"
#include <type_traits>
#include "SkrBase/unicode/unicode_algo.hpp"

namespace skr::container
{
// TODO. support eachable container
// TODO. 默认使用 buffer index，如果需要使用 codepoint index 需要通过转换 API
// TODO. 需要使用 concept 来替代 EachAbleContainer
// TODO. string api 的三种形式
//  1. 单字符（code point）操作
//  2. c 串操作
//  3. 容器操作
// TODO. literal 切换操作
template <typename Memory>
struct U8String : protected Memory {
    // from memory
    using typename Memory::DataType;
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;

    // data ref
    using DataRef  = StringDataRef<DataType, SizeType, false>;
    using CDataRef = StringDataRef<DataType, SizeType, true>;

    // cursor & iterator

    static_assert(std::is_same_v<DataType, skr_char8>, "U8String only supports char8_t");

    // ctor & dtor
    U8String() noexcept;
    U8String(const DataType* str) noexcept;
    U8String(const DataType* str, SizeType len) noexcept;
    ~U8String();

    // factory
    static U8String Raw(const char* str) noexcept;
    static U8String Wide(const wchar_t* str) noexcept;
    static U8String Utf8(const skr_char8* str) noexcept;
    static U8String Utf16(const skr_char16* str) noexcept;
    static U8String Utf32(const skr_char32* str) noexcept;

    // copy & move
    U8String(const U8String& other);
    U8String(U8String&& other) noexcept;

    // assign & move assign
    U8String& operator=(const U8String& rhs);
    U8String& operator=(U8String&& rhs) noexcept;

    // special assign
    void assign(const DataType* str) noexcept;
    void assign(const DataType* str, SizeType len) noexcept;
    template <EachAbleContainer U>
    void assign(U&& container) noexcept;

    // compare
    bool operator==(const U8String& rhs) const noexcept;
    bool operator!=(const U8String& rhs) const noexcept;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    bool            is_empty() const;
    DataType*       data();
    const DataType* data() const;
    Memory&         memory();
    const Memory&   memory() const;

    // str getter
    SizeType        length_text() const;
    SizeType        length_buffer() const;
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

    // append
    DataRef append(const DataType* str);
    DataRef append(const DataType* str, SizeType len);
    DataRef append(UTF8Seq seq);
    DataRef append(UTF16Seq seq);
    DataRef append(skr_char32 ch);
    template <EachAbleContainer U>
    DataRef append(const U& str);

    // append at
    DataRef append_at(SizeType idx, const DataType* str);
    DataRef append_at(SizeType idx, const DataType* str, SizeType len);
    DataRef append_at(SizeType idx, UTF8Seq seq);
    DataRef append_at(SizeType idx, UTF16Seq seq);
    DataRef append_at(SizeType idx, skr_char32 ch);
    template <EachAbleContainer U>
    DataRef append_at(SizeType idx, const U& str);

    // operator append
    DataRef operator+=(const U8String& str);
    DataRef operator+=(UTF8Seq seq);
    DataRef operator+=(UTF16Seq seq);
    DataRef operator+=(skr_char32 ch);
    template <EachAbleContainer U>
    DataRef operator+=(const U& str);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    bool remove(skr_char32 ch);
    bool remove_last(skr_char32 ch);
    bool remove_all(skr_char32 ch);
    template <EachAbleContainer U>
    bool remove(const U& str);
    template <EachAbleContainer U>
    bool remove_last(const U& str);
    template <EachAbleContainer U>
    SizeType remove_all(const U& str);

    // replace
    // index & modify
    // find
    // contains
    // sub string
    // trim
    // split

    // syntax
    const U8String& readonly() const;
};
} // namespace skr::container

namespace skr::container
{

}
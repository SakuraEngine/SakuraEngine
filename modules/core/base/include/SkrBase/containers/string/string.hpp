#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include <type_traits>
#include "SkrBase/containers/string/string_view.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include "SkrBase/unicode/unicode_iterator.hpp"

namespace skr::container
{
// TODO. 对有 const 二义性的 API 中非 const 部分添加 _w 后缀来保证普通操作不会触发常量串的拷贝
// TODO. 对需要输入字符串的 API 提供 StringView/Seq 两种版本
// TODO. 对需要使用串容器的 API 提供 EachAbleContainer 版本
// TODO. 一些字符串工具函数
//  isalnum 全是字符或者数字
//  isalpha 全是字符
//  isdigit 是否只包含数字字符
//  islower 是否只包含小写字符
//  isupper 是否只包含大写字符
// TODO. 是否提供 ToStringTraits 用于将各种类型转化为字符串
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
    using Cursor    = UTF8Cursor<SizeType, false>;
    using CCursor   = UTF8Cursor<SizeType, true>;
    using Iter      = UTF8Iter<SizeType, false>;
    using CIter     = UTF8Iter<SizeType, true>;
    using IterInv   = UTF8IterInv<SizeType, true>;
    using CIterInv  = UTF8IterInv<SizeType, true>;
    using Range     = UTF8Range<SizeType, false>;
    using CRange    = UTF8Range<SizeType, true>;
    using RangeInv  = UTF8RangeInv<SizeType, true>;
    using CRangeInv = UTF8RangeInv<SizeType, true>;

    // other types
    using ViewType        = U8StringView<SizeType>;
    using PartitionResult = StringPartitionResult<ViewType>;

    // helper
    using CharTraits               = std::char_traits<DataType>;
    static constexpr SizeType npos = npos_of<SizeType>;

    // traits
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

    // join & build factory
    // TODO. join:  针对容器的拼接
    // TODO. build: 相当于 + 运算符，用于连接一系列可以被转化为 string 的对象以构建字符串

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
    DataRef append(ViewType view);
    DataRef append(UTF8Seq seq);
    template <EachAbleContainer U>
    DataRef append(const U& str);

    // append at
    DataRef append_at(SizeType idx, const DataType* str);
    DataRef append_at(SizeType idx, const DataType* str, SizeType len);
    DataRef append_at(SizeType idx, ViewType view);
    DataRef append_at(SizeType idx, UTF8Seq seq);
    template <EachAbleContainer U>
    DataRef append_at(SizeType idx, const U& str);

    // TODO. remove_at & remove & remove_last & remove_all
    // TODO. replace (copy)
    // TODO. index & modify (write API)
    // TODO. sub_string & first & last (copy)
    // TODO. find (write API)
    // TODO. contains & count
    // TODO. starts & ends
    // TODO. remove prefix & suffix (copy)
    // TODO. trim (copy)
    // TODO. trim_invalid (copy)
    // TODO. split
    // TODO. partition

    // text index
    SizeType buffer_index_to_text(SizeType index) const;
    SizeType text_index_to_buffer(SizeType index) const;

    // syntax
    const U8String& readonly() const;
    ViewType        view() const;
};
} // namespace skr::container

namespace skr::container
{

}
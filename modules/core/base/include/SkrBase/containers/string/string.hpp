#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/string/string_def.hpp"
#include <type_traits>
#include "SkrBase/unicode/unicode_algo.hpp"

namespace skr::container
{
// TODO. 类似 COW 的 literal 切换操作
// TODO. 通过万能的 view 转换来替代 EachAbleContainer, 暂时不支持异种编码的转换，因为一般输入的
// TODO. 基本操作默认返回新串，对自身的操作使用 xxx_self 来进行
// TODO. 一些字符串工具函数
//  isalnum 全是字符或者数字
//  isalpha 全是字符
//  isdigit 是否只包含数字字符
//  islower 是否只包含小写字符
//  isupper 是否只包含大写字符
//  join 连接字符串
//  partition 对半分割字符串
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

    // helper
    static constexpr SizeType npos = npos_of<SizeType>;

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
    template <EachAbleContainer U>
    bool replace(const U& dst, const U& src, SizeType start = 0, SizeType count = npos);

    // index & modify
    const DataType& at_buffer(SizeType index) const;
    DataType&       at_buffer_w(SizeType index);
    const DataType& last_buffer(SizeType index) const;
    DataType&       last_buffer_w(SizeType index);
    UTF8Seq         at_text(SizeType index) const;
    UTF8Seq         last_text(SizeType index) const;

    // find
    template <EachAbleContainer U>
    CDataRef find(const U& pattern, SizeType start = 0, SizeType count = npos) const;
    template <EachAbleContainer U>
    CDataRef find_last(const U& pattern, SizeType start = 0, SizeType count = npos) const;
    template <EachAbleContainer U>
    DataRef find_w(const U& pattern, SizeType start = 0, SizeType count = npos) const;
    template <EachAbleContainer U>
    DataRef find_last_w(const U& pattern, SizeType start = 0, SizeType count = npos) const;

    // contains & count
    template <EachAbleContainer U>
    bool contains(const U& pattern) const;
    template <EachAbleContainer U>
    bool count(const U& pattern) const;

    // starts & ends
    template <EachAbleContainer U>
    bool starts_with(const U& prefix) const;
    template <EachAbleContainer U>
    bool ends_with(const U& suffix) const;

    // sub string

    // trim
    template <EachAbleContainer U>
    U8String trim(const U& characters) noexcept;
    template <EachAbleContainer U>
    U8String trim_start(const U& characters) noexcept;
    template <EachAbleContainer U>
    U8String trim_end(const U& characters) noexcept;
    template <EachAbleContainer U>
    void trim_self(const U& characters) noexcept;
    template <EachAbleContainer U>
    void trim_start_self(const U& characters) noexcept;
    template <EachAbleContainer U>
    void trim_end_self(const U& characters) noexcept;

    // split
    // TODO. split iter
    template <EachAbleContainer U, typename Out>
    SizeType split(const U& splitter, Out& pieces, bool cull_empty = true) const;

    // text index
    SizeType buffer_index_to_text(SizeType index) const;
    SizeType text_index_to_buffer(SizeType index) const;

    // syntax
    const U8String& readonly() const;
};
} // namespace skr::container

namespace skr::container
{

}
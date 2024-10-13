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
    U8String(AllocatorCtorParam param = {}) noexcept;
    U8String(const DataType* str, AllocatorCtorParam param = {}) noexcept;
    U8String(const DataType* str, SizeType len, AllocatorCtorParam param = {}) noexcept;
    U8String(ViewType view, AllocatorCtorParam param = {}) noexcept;
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
    bool operator==(ViewType view) const noexcept;
    bool operator!=(ViewType view) const noexcept;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    bool            is_empty() const;
    DataType*       data_w();
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
    void release(SizeType reserve_capacity = 0); // TODO. release test when equal to capacity FOR ALL CONTAINERS
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
    DataRef append_at(SizeType idx, const DataType* str);
    DataRef append_at(SizeType idx, const DataType* str, SizeType len);
    DataRef append_at(SizeType idx, ViewType view);
    DataRef append_at(SizeType idx, UTF8Seq seq);
    template <EachAbleContainer U>
    DataRef append_at(SizeType idx, const U& container);

    // remove
    // TODO. remove based on replace
    void     remove_at(SizeType index, SizeType n);
    bool     remove(ViewType view);
    bool     remove_last(ViewType view);
    bool     remove_all(ViewType view);
    bool     remove(UTF8Seq seq);
    bool     remove_last(UTF8Seq seq);
    bool     remove_all(UTF8Seq seq);
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
    void     replace_range_copy(ViewType to, SizeType start = 0, SizeType count = npos) const;

    // index & modify
    const DataType& at_buffer(SizeType index) const;
    const DataType& last_buffer(SizeType index) const;
    DataType&       at_buffer_w(SizeType index);
    DataType&       last_buffer_w(SizeType index);
    UTF8Seq         at_text(SizeType index) const;
    UTF8Seq         last_text(SizeType index) const;
    // if you want to modify seq (code point), use replace or remove

    // sub_string
    void     first(SizeType count);
    void     last(SizeType count);
    void     substr(SizeType start, SizeType count = npos);
    U8String first_copy(SizeType count) const;
    U8String last_copy(SizeType count) const;
    U8String substr_copy(SizeType start, SizeType count = npos) const;
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
    void     remove_prefix(const ViewType& prefix);
    void     remove_suffix(const ViewType& suffix);
    void     remove_prefix(const UTF8Seq& prefix);
    void     remove_suffix(const UTF8Seq& suffix);
    U8String remove_prefix_copy(const ViewType& prefix) const;
    U8String remove_suffix_copy(const ViewType& suffix) const;
    U8String remove_prefix_copy(const UTF8Seq& prefix) const;
    U8String remove_suffix_copy(const UTF8Seq& suffix) const;

    // trim
    void     trim(const ViewType& characters);
    void     trim_start(const ViewType& characters);
    void     trim_end(const ViewType& characters);
    void     trim(const UTF8Seq& ch);
    void     trim_start(const UTF8Seq& ch);
    void     trim_end(const UTF8Seq& ch);
    U8String trim_copy(const ViewType& characters) const;
    U8String trim_start_copy(const ViewType& characters) const;
    U8String trim_end_copy(const ViewType& characters) const;
    U8String trim_copy(const UTF8Seq& ch) const;
    U8String trim_start_copy(const UTF8Seq& ch) const;
    U8String trim_end_copy(const UTF8Seq& ch) const;

    // trim_invalid
    void     trim_invalid();
    void     trim_invalid_start();
    void     trim_invalid_end();
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

    // text index
    SizeType buffer_index_to_text(SizeType index) const;
    SizeType text_index_to_buffer(SizeType index) const;

    // syntax
    const U8String& readonly() const;
    ViewType        view() const;
    bool            force_cancel_literal() const;

private:
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
inline U8String<Memory>::SizeType U8String<Memory>::_grow(SizeType grow_size)
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
            memory::copy(_data(), str, len);
        }
    }
}
template <typename Memory>
inline U8String<Memory>::DataType* U8String<Memory>::_data()
{
    return Memory::data();
}
template <typename Memory>
inline const U8String<Memory>::DataType* U8String<Memory>::_data() const
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
    _assign_with_literal_check(str, CharTraits::length(str));
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
inline U8String<Memory>::~U8String()
{
    // handled in memory
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
    _assign_with_literal_check(str, CharTraits::length(str));
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
        memory::copy(_data(), p, n);
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
inline bool U8String<Memory>::operator==(ViewType view) const noexcept
{
    return this->view() == view;
}
template <typename Memory>
inline bool U8String<Memory>::operator!=(ViewType view) const noexcept
{
    return this->view() != view;
}

// getter
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::slack() const
{
    return capacity() - size();
}
template <typename Memory>
inline bool U8String<Memory>::is_empty() const
{
    return size() == 0;
}
template <typename Memory>
inline U8String<Memory>::DataType* U8String<Memory>::data_w()
{
    _pre_modify();
    return _data();
}
template <typename Memory>
inline const U8String<Memory>::DataType* U8String<Memory>::data() const
{
    return _data();
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
inline U8String<Memory>::SizeType U8String<Memory>::length_text() const
{
    return view().length_text();
}
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::length_buffer() const
{
    return view().length_buffer();
}
template <typename Memory>
inline const U8String<Memory>::DataType* U8String<Memory>::c_str() const
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
        _realloc(reserve_capacity);
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
        memory::destruct(_data() + expect_size, size() - expect_size);
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
        memory::destruct(_data() + expect_size, size() - expect_size);
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
        memory::construct(_data() + size(), expect_size - size());
    }
    else if (expect_size < size())
    {
        memory::destruct(_data() + expect_size, size() - expect_size);
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
        memory::destruct(_data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::add(const DataType& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index(); i < size(); ++i)
    {
        new (_data() + i) DataType(v);
    }
    return ref;
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::add_unsafe(SizeType n)
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
inline U8String<Memory>::DataRef U8String<Memory>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    memory::construct(ref.ptr(), n);
    return ref;
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::add_zeroed(SizeType n)
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
    memory::move(_data() + idx + n, _data() + idx, move_n);
}
template <typename Memory>
inline void U8String<Memory>::add_at_default(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    memory::construct(_data() + idx, n);
}
template <typename Memory>
inline void U8String<Memory>::add_at_zeroed(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    std::memset(_data() + idx, 0, n * sizeof(DataType));
}

// append
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append(const DataType* str)
{
    return append(str, CharTraits::length(str));
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append(const DataType* str, SizeType len)
{
    DataRef ref = add_unsafe(len);
    memory::copy(ref.ptr(), str, len);
    return ref;
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append(ViewType view)
{
    return append(view.data(), view.size());
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append(UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return append(&seq.data[0], seq.len);
    }
    return _data() ? DataRef(_data() + size(), size()) : DataRef();
}
template <typename Memory>
template <EachAbleContainer U>
inline U8String<Memory>::DataRef U8String<Memory>::append(const U& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        if (n)
        {
            DataRef ref = add_unsafe(n);
            memory::copy(ref.ptr(), p, n);
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
inline U8String<Memory>::DataRef U8String<Memory>::append_at(SizeType idx, const DataType* str)
{
    return append_at(idx, str, CharTraits::length(str));
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append_at(SizeType idx, const DataType* str, SizeType len)
{
    if (len)
    {
        add_at_unsafe(idx, len);
        memory::copy(_data() + idx, str, len);
    }
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append_at(SizeType idx, ViewType view)
{
    return append_at(idx, view.data(), view.size());
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::append_at(SizeType idx, UTF8Seq seq)
{
    if (seq.is_valid())
    {
        return append_at(idx, &seq.data[0], seq.len);
    }
    return _data() ? DataRef(_data() + size(), size()) : DataRef();
}
template <typename Memory>
template <EachAbleContainer U>
inline U8String<Memory>::DataRef U8String<Memory>::append_at(SizeType idx, const U& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        if (n)
        {
            add_at_unsafe(idx, n);
            memory::copy(_data() + idx, p, n);
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
        memory::destruct(_data() + index, n);

        // move data
        if (move_n)
        {
            memory::move(_data() + index, _data() + size() - move_n, move_n);
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
        remove_at(ref.index, view.size());
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
inline bool U8String<Memory>::remove_all(ViewType view)
{
    if (!is_empty())
    {
        CDataRef first_found = find(view);

        if (first_found)
        {
            DataType* write     = _data();
            DataType* read      = _data();
            SizeType  read_size = size();

            _pre_modify();
            DataRef found = first_found;
            while (found)
            {
                // copy data before found
                if (write != read)
                {
                    memory::move(write, read, found.ptr() - read);
                }

                // update read info
                read_size = read_size - (found.ptr() - read) - view.size();
                read      = found.ptr() + view.size();

                // find next
                found = ViewType{ read, read_size }.find(view);
            }

            // copy final data
            memory::move(write, read, read_size);

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
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
inline bool U8String<Memory>::remove_all(UTF8Seq seq)
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
    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_last_copy(ViewType view) const
{
    U8String result = *this;
    result.remove_last(view);
    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_all_copy(ViewType view) const
{
    U8String result = *this;
    result.remove_all(view);
    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove(seq);
    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_last_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove_last(seq);
    return result;
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_all_copy(UTF8Seq seq) const
{
    U8String result = *this;
    result.remove_all(seq);
    return result;
}

// replace
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::replace(ViewType from, ViewType to, SizeType start, SizeType count)
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
        _pre_modify();

        DataRef found_ref = find_view.find(from);
        while (found_ref)
        {
            // copy data
            memory::copy(found_ref.ptr(), to.data(), to.size());

            // update view
            find_view = find_view.subview(found_ref.index() + to.size());

            // find next
            found_ref = find_view.find(from);
        }
    }
    else if (to.size() < from.size())
    { // replace from start to end
        _pre_modify();

        DataRef   found_ref = find_view.find(from);
        DataType* read      = found_ref.ptr();
        DataType* write     = found_ref.ptr();
        while (found_ref)
        {
            // move data before found
            if (write != read)
            {
                auto move_size = found_ref.ptr() - read;
                memory::move(write, read, move_size);
                write += move_size;
            }

            // copy replace part
            memory::copy(write, to.data(), to.size());
            write += to.size();

            // update read and find view
            find_view = find_view.subview(found_ref.index() + from.size());

            // update read
            read = find_view.data();

            // find next
            found_ref = find_view.find(from);
        }

        // move final part
        DataType* end = _data() + size();
        if (read != end)
        {
            memory::move(write, read, end - read);
        }

        // update size
        _set_size(size() - replace_size_from + replace_size_to);
    }
    else
    { // make a new string and copy
        U8String result = replace_copy(from, to, start, count);
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::replace_copy(ViewType from, ViewType to, SizeType start, SizeType count) const
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

    if (to.size() <= from.size())
    {
        U8String result = *this;
        result.replace(from, to, start, count);
    }
    else
    {
        U8String result;
        result.reserve(count() + replace_size_to - replace_size_from);

        CDataRef        found_ref = find_view.find(from);
        const DataType* read      = _data();

        while (found_ref)
        {
            // copy data before found
            result.append(ViewType{ read, found_ref.ptr() - read });

            // copy replace part
            result.append(ViewType{ to.data(), to.size() });

            // update read and find view
            find_view = find_view.subview(found_ref.index() + from.size());
            read      = find_view.data();

            // find next
            found_ref = find_view.find(from);
        }

        // copy final part
        result.append(ViewType{ read, _data() + size() - read });
    }
}
template <typename Memory>
inline void U8String<Memory>::replace_range(ViewType to, SizeType start, SizeType count)
{
    SKR_ASSERT(start <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == npos || count <= (size() - start) && "undefined behaviour exceeding size of string view");

    if (count == 0)
    {
        return;
    }

    if (to.size() <= count)
    {
        _pre_modify();

        // copy to
        memory::copy(_data() + start, to._data(), to.size());

        // move rest
        memory::move(_data() + start + to.size(), _data() + start + count, size() - start - count);
    }
    else
    {
        SizeType reserve_size = size() + to.size() - count;
        _pre_modify(reserve_size);
        reserve(reserve_size);

        // move rest
        memory::move(_data() + start + to.size(), _data() + start + count, size() - start - count);

        // copy to
        memory::copy(_data() + start, to._data(), to.size());
    }

    // update size
    _set_size(size() - count + to.size());
}
template <typename Memory>
inline void U8String<Memory>::replace_range_copy(ViewType to, SizeType start, SizeType count) const
{
    U8String result = *this;
    result.replace_range(to, start, count);
}

// index & modify
template <typename Memory>
inline const U8String<Memory>::DataType& U8String<Memory>::at_buffer(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    return _data()[index];
}
template <typename Memory>
inline const U8String<Memory>::DataType& U8String<Memory>::last_buffer(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    return _data()[size() - index - 1];
}
template <typename Memory>
inline U8String<Memory>::DataType& U8String<Memory>::at_buffer_w(SizeType index)
{
    SKR_ASSERT(is_valid_index(index) && "undefined behaviour accessing out of bounds");
    _pre_modify();
    return _data()[index];
}
template <typename Memory>
inline U8String<Memory>::DataType& U8String<Memory>::last_buffer_w(SizeType index)
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
    if (start == 0 && count == size())
    {
        return;
    }
    memory::move(_data(), _data() + start, count);
    _set_size(count);
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
template <typename Memory>
inline U8String<Memory>::ViewType U8String<Memory>::first_view(SizeType count) const
{
    return view().first(count);
}
template <typename Memory>
inline U8String<Memory>::ViewType U8String<Memory>::last_view(SizeType count) const
{
    return view().last(count);
}
template <typename Memory>
inline U8String<Memory>::ViewType U8String<Memory>::subview(SizeType start, SizeType count) const
{
    return view().subview(start, count);
}

// find
template <typename Memory>
inline U8String<Memory>::CDataRef U8String<Memory>::find(const ViewType& pattern) const
{
    return view().find(pattern);
}
template <typename Memory>
inline U8String<Memory>::CDataRef U8String<Memory>::find_last(const ViewType& pattern) const
{
    return view().find_last(pattern);
}
template <typename Memory>
inline U8String<Memory>::CDataRef U8String<Memory>::find(const UTF8Seq& pattern) const
{
    return view().find(pattern);
}
template <typename Memory>
inline U8String<Memory>::CDataRef U8String<Memory>::find_last(const UTF8Seq& pattern) const
{
    return view().find_last(pattern);
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::find_w(const ViewType& pattern)
{
    _pre_modify();
    return view().find(pattern);
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::find_last_w(const ViewType& pattern)
{
    _pre_modify();
    return view().find_last(pattern);
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::find_w(const UTF8Seq& pattern)
{
    _pre_modify();
    return view().find(pattern);
}
template <typename Memory>
inline U8String<Memory>::DataRef U8String<Memory>::find_last_w(const UTF8Seq& pattern)
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
inline U8String<Memory>::SizeType U8String<Memory>::count(const ViewType& pattern) const
{
    return view().count(pattern);
}
template <typename Memory>
inline bool U8String<Memory>::contains(const UTF8Seq& pattern) const
{
    return view().contains(pattern);
}
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::count(const UTF8Seq& pattern) const
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
inline void U8String<Memory>::remove_prefix(const ViewType& prefix)
{
    if (starts_with(prefix))
    {
        remove_at(0, prefix.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::remove_suffix(const ViewType& suffix)
{
    if (ends_with(suffix))
    {
        remove_at(size() - suffix.size(), suffix.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::remove_prefix(const UTF8Seq& prefix)
{
    if (starts_with(prefix))
    {
        remove_at(0, prefix.len);
    }
}
template <typename Memory>
inline void U8String<Memory>::remove_suffix(const UTF8Seq& suffix)
{
    if (ends_with(suffix))
    {
        remove_at(size() - suffix.len, suffix.len);
    }
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_prefix_copy(const ViewType& prefix) const
{
    return { view().remove_prefix_copy(prefix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_suffix_copy(const ViewType& suffix) const
{
    return { view().remove_suffix_copy(suffix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_prefix_copy(const UTF8Seq& prefix) const
{
    return { view().remove_prefix_copy(prefix) };
}
template <typename Memory>
inline U8String<Memory> U8String<Memory>::remove_suffix_copy(const UTF8Seq& suffix) const
{
    return { view().remove_suffix_copy(suffix) };
}

// trim
template <typename Memory>
inline void U8String<Memory>::trim(const ViewType& characters)
{
    trim_start(characters);
    trim_end(characters);
}
template <typename Memory>
inline void U8String<Memory>::trim_start(const ViewType& characters)
{
    ViewType trim_view = view().trim_start(characters);
    if (trim_view.size() != size())
    {
        _pre_modify();
        memory::move(_data(), trim_view._data(), trim_view.size());
        _set_size(trim_view.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::trim_end(const ViewType& characters)
{
    ViewType trim_view = view().trim_end(characters);
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::trim(const UTF8Seq& ch)
{
    trim_start(ch);
    trim_end(ch);
}
template <typename Memory>
inline void U8String<Memory>::trim_start(const UTF8Seq& ch)
{
    ViewType trim_view = view().trim_start(ch);
    if (trim_view.size() != size())
    {
        _pre_modify();
        memory::move(_data(), trim_view._data(), trim_view.size());
        _set_size(trim_view.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::trim_end(const UTF8Seq& ch)
{
    ViewType trim_view = view().trim_end(ch);
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
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
inline void U8String<Memory>::trim_invalid()
{
    trim_invalid_start();
    trim_invalid_end();
}
template <typename Memory>
inline void U8String<Memory>::trim_invalid_start()
{
    ViewType trim_view = view().trim_invalid_start();
    if (trim_view.size() != size())
    {
        _pre_modify();
        memory::move(_data(), trim_view._data(), trim_view.size());
        _set_size(trim_view.size());
    }
}
template <typename Memory>
inline void U8String<Memory>::trim_invalid_end()
{
    ViewType trim_view = view().trim_invalid_end();
    if (trim_view.size() != size())
    {
        _pre_modify();
        _set_size(trim_view.size());
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
inline U8String<Memory>::PartitionResult U8String<Memory>::partition(const ViewType& delimiter) const
{
    return view().partition(delimiter);
}
template <typename Memory>
inline U8String<Memory>::PartitionResult U8String<Memory>::partition(const UTF8Seq& delimiter) const
{
    return view().partition(delimiter);
}

// split
template <typename Memory>
template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
inline U8String<Memory>::SizeType U8String<Memory>::split(Buffer& out, const ViewType& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split(out, delimiter, cull_empty, limit);
}
template <typename Memory>
template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
inline U8String<Memory>::SizeType U8String<Memory>::split_each(F&& func, const ViewType& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split_each(std::forward<F>(func), delimiter, cull_empty, limit);
}
template <typename Memory>
template <CanAdd<const U8StringView<typename Memory::SizeType>&> Buffer>
inline U8String<Memory>::SizeType U8String<Memory>::split(Buffer& out, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split(out, delimiter, cull_empty, limit);
}
template <typename Memory>
template <std::invocable<const U8StringView<typename Memory::SizeType>&> F>
inline U8String<Memory>::SizeType U8String<Memory>::split_each(F&& func, const UTF8Seq& delimiter, bool cull_empty, SizeType limit) const
{
    return view().split_each(std::forward<F>(func), delimiter, cull_empty, limit);
}

// text index
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::buffer_index_to_text(SizeType index) const
{
    return view().buffer_index_to_text(index);
}
template <typename Memory>
inline U8String<Memory>::SizeType U8String<Memory>::text_index_to_buffer(SizeType index) const
{
    return view().text_index_to_buffer(index);
}

// syntax
template <typename Memory>
inline const U8String<Memory>& U8String<Memory>::readonly() const
{
    return *this;
}
template <typename Memory>
inline U8String<Memory>::ViewType U8String<Memory>::view() const
{
    return { _data(), size() };
}
template <typename Memory>
inline bool U8String<Memory>::force_cancel_literal() const
{
    return _pre_modify();
}
} // namespace skr::container
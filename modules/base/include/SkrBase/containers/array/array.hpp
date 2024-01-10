#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"
#include "SkrBase/algo/remove.hpp"
#include "SkrBase/containers/array/array_def.hpp"
#include "SkrBase/containers/array/array_iterator.hpp"

// Array def
namespace skr::container
{
template <typename Memory>
struct Array : protected Memory {
    // from memory
    using typename Memory::DataType;
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;

    // data ref
    using DataRef  = ArrayDataRef<DataType, SizeType, false>;
    using CDataRef = ArrayDataRef<DataType, SizeType, true>;

    // cursor & iterator
    using Cursor   = ArrayCursor<DataType, SizeType, false>;
    using CCursor  = ArrayCursor<DataType, SizeType, true>;
    using Iter     = ArrayIter<DataType, SizeType, false>;
    using CIter    = ArrayIter<DataType, SizeType, true>;
    using IterInv  = ArrayIterInv<DataType, SizeType, false>;
    using CIterInv = ArrayIterInv<DataType, SizeType, true>;

    // stl iterator
    using StlIt  = DataType*;
    using CStlIt = const DataType*;

    // ctor & dtor
    Array(AllocatorCtorParam param = {}) noexcept;
    Array(SizeType size, AllocatorCtorParam param = {}) noexcept;
    Array(SizeType size, const DataType& v, AllocatorCtorParam param = {}) noexcept;
    Array(const DataType* p, SizeType n, AllocatorCtorParam param = {}) noexcept;
    Array(std::initializer_list<DataType> init_list, AllocatorCtorParam param = {}) noexcept;
    ~Array();

    // copy & move
    Array(const Array& other);
    Array(Array&& other) noexcept;

    // assign & move assign
    Array& operator=(const Array& rhs);
    Array& operator=(Array&& rhs) noexcept;

    // special assign
    void assign(const DataType* p, SizeType n);
    void assign(std::initializer_list<DataType> init_list);

    // compare
    bool operator==(const Array& rhs) const;
    bool operator!=(const Array& rhs) const;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    bool            empty() const;
    DataType*       data();
    const DataType* data() const;
    Memory&         memory();
    const Memory&   memory() const;

    // validate
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const DataType* p) const;

    // memory op
    void clear();
    void release(SizeType reserve_capacity = 0);
    void reserve(SizeType expect_capacity);
    void shrink();
    void resize(SizeType expect_size, const DataType& new_value);
    void resize(SizeType expect_size);
    void resize_unsafe(SizeType expect_size);
    void resize_default(SizeType expect_size);
    void resize_zeroed(SizeType expect_size);

    // add
    DataRef add(const DataType& v, SizeType n = 1);
    DataRef add(DataType&& v);
    DataRef add_unique(const DataType& v);
    DataRef add_unsafe(SizeType n = 1);
    DataRef add_default(SizeType n = 1);
    DataRef add_zeroed(SizeType n = 1);

    // add at
    void add_at(SizeType idx, const DataType& v, SizeType n = 1);
    void add_at(SizeType idx, DataType&& v);
    void add_at_unsafe(SizeType idx, SizeType n = 1);
    void add_at_default(SizeType idx, SizeType n = 1);
    void add_at_zeroed(SizeType idx, SizeType n = 1);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    void emplace_at(SizeType index, Args&&... args);

    // append
    DataRef append(const Array& arr);
    DataRef append(std::initializer_list<DataType> init_list);
    template <typename U = DataType>
    DataRef append(const U* p, SizeType n);

    // append at
    void append_at(SizeType idx, const Array& arr);
    void append_at(SizeType idx, std::initializer_list<DataType> init_list);
    template <typename U = DataType>
    void append_at(SizeType idx, const U* p, SizeType n);

    // operator append
    DataRef operator+=(const DataType& v);
    DataRef operator+=(DataType&& v);
    DataRef operator+=(std::initializer_list<DataType> init_list);
    DataRef operator+=(const Array& arr);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    void remove_at_swap(SizeType index, SizeType n = 1);
    template <typename U = DataType>
    bool remove(const U& v);
    template <typename U = DataType>
    bool remove_swap(const U& v);
    template <typename U = DataType>
    bool remove_last(const U& v);
    template <typename U = DataType>
    bool remove_last_swap(const U& v);
    template <typename U = DataType>
    SizeType remove_all(const U& v);
    template <typename U = DataType>
    SizeType remove_all_swap(const U& v);

    // remove if
    template <typename Pred>
    bool remove_if(Pred&& pred);
    template <typename Pred>
    bool remove_if_swap(Pred&& pred);
    template <typename Pred>
    bool remove_last_if(Pred&& pred);
    template <typename Pred>
    bool remove_last_if_swap(Pred&& pred);
    template <typename Pred>
    SizeType remove_all_if(Pred&& pred);
    template <typename Pred>
    SizeType remove_all_if_swap(Pred&& pred);

    // erase
    StlIt  erase(const StlIt& it);
    CStlIt erase(const CStlIt& it);
    StlIt  erase_swap(const StlIt& it);
    CStlIt erase_swap(const CStlIt& it);

    // modify
    DataType&       operator[](SizeType index);
    const DataType& operator[](SizeType index) const;
    DataType&       at(SizeType index);
    const DataType& at(SizeType index) const;
    DataType&       last(SizeType index = 0);
    const DataType& last(SizeType index = 0) const;

    // front/back
    DataType&       front();
    const DataType& front() const;
    DataType&       back();
    const DataType& back() const;
    void            push_back(const DataType& v);
    void            push_back(DataType&& v);
    void            pop_back();
    DataType&       pop_back_get();

    // find
    template <typename U = DataType>
    DataRef find(const U& v);
    template <typename U = DataType>
    DataRef find_last(const U& v);
    template <typename U = DataType>
    CDataRef find(const U& v) const;
    template <typename U = DataType>
    CDataRef find_last(const U& v) const;

    // find if
    template <typename Pred>
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <typename U = DataType>
    bool contains(const U& v) const;
    template <typename Pred>
    bool contains_if(Pred&& pred) const;
    template <typename U = DataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    SizeType count_if(Pred&& pred) const;

    // sort
    template <typename Functor = Less<DataType>>
    void sort(Functor&& f = {});
    template <typename Functor = Less<DataType>>
    void sort_stable(Functor&& f = {});

    // support heap
    DataType& heap_top();
    template <typename Functor = Less<DataType>>
    void heapify(Functor&& f = {});
    template <typename Functor = Less<DataType>>
    bool is_heap(Functor&& f = {});
    template <typename Functor = Less<DataType>>
    SizeType heap_push(DataType&& v, Functor&& f = {});
    template <typename Functor = Less<DataType>>
    SizeType heap_push(const DataType& v, Functor&& f = {});
    template <typename Functor = Less<DataType>>
    void heap_pop(Functor&& f = {});
    template <typename Functor = Less<DataType>>
    DataType heap_pop_get(Functor&& f = {});
    template <typename Functor = Less<DataType>>
    void heap_remove_at(SizeType index, Functor&& f = {});
    template <typename Functor = Less<DataType>>
    void heap_sort(Functor&& f = {});

    // support stack
    void            stack_pop(SizeType n = 1);
    void            stack_pop_unsafe(SizeType n = 1);
    DataType        stack_pop_get();
    void            stack_push(const DataType& v);
    void            stack_push(DataType&& v);
    DataType&       stack_top();
    const DataType& stack_top() const;
    DataType&       stack_bottom();
    const DataType& stack_bottom() const;

    // cursor & iter
    Cursor   cursor_begin();
    CCursor  cursor_begin() const;
    Cursor   cursor_end();
    CCursor  cursor_end() const;
    Iter     iter();
    CIter    iter() const;
    IterInv  iter_inv();
    CIterInv iter_inv() const;

    // support foreach
    StlIt  begin();
    StlIt  end();
    CStlIt begin() const;
    CStlIt end() const;

    // syntax
    const Array& readonly() const;

private:
    // helper
    void _realloc(SizeType expect_capacity);
    void _free();
    void _set_size(SizeType new_size);
};
} // namespace skr::container

// Array impl
namespace skr::container
{
// helper
template <typename Memory>
SKR_INLINE void Array<Memory>::_realloc(SizeType expect_capacity)
{
    Memory::realloc(expect_capacity);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
SKR_INLINE void Array<Memory>::_set_size(SizeType new_size)
{
    Memory::set_size(new_size);
}

// ctor & dtor
template <typename Memory>
SKR_INLINE Array<Memory>::Array(AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
}
template <typename Memory>
SKR_INLINE Array<Memory>::Array(SizeType size, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize(size);
}
template <typename Memory>
SKR_INLINE Array<Memory>::Array(SizeType size, const DataType& v, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize(size, v);
}
template <typename Memory>
SKR_INLINE Array<Memory>::Array(const DataType* p, SizeType n, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(n);
    memory::copy(data(), p, n);
}
template <typename Memory>
SKR_INLINE Array<Memory>::Array(std::initializer_list<DataType> init_list, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(init_list.size());
    memory::copy(data(), init_list.begin(), init_list.size());
}
template <typename Memory>
SKR_INLINE Array<Memory>::~Array()
{
    // handled in memory
}

// copy & move
template <typename Memory>
SKR_INLINE Array<Memory>::Array(const Array& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE Array<Memory>::Array(Array&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
SKR_INLINE Array<Memory>& Array<Memory>::operator=(const Array& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE Array<Memory>& Array<Memory>::operator=(Array&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// special assign
template <typename Memory>
SKR_INLINE void Array<Memory>::assign(const DataType* p, SizeType n)
{
    // clear and resize
    clear();
    resize_unsafe(n);

    // copy items
    memory::copy(data(), p, n);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::assign(std::initializer_list<DataType> init_list)
{
    assign(init_list.begin(), init_list.size());
}

// compare
template <typename Memory>
SKR_INLINE bool Array<Memory>::operator==(const Array& rhs) const
{
    return size() == rhs.size() && memory::compare(data(), rhs.data(), size());
}
template <typename Memory>
SKR_INLINE bool Array<Memory>::operator!=(const Array& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename Memory>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::slack() const
{
    return capacity() - size();
}
template <typename Memory>
SKR_INLINE bool Array<Memory>::empty() const
{
    return size() == 0;
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType* Array<Memory>::data()
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType* Array<Memory>::data() const
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE Memory& Array<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& Array<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool Array<Memory>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < size();
}
template <typename Memory>
SKR_INLINE bool Array<Memory>::is_valid_pointer(const DataType* p) const
{
    return p >= begin() && p < end();
}

// memory op
template <typename Memory>
SKR_INLINE void Array<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
SKR_INLINE void Array<Memory>::release(SizeType reserve_capacity)
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
SKR_INLINE void Array<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename Memory>
SKR_INLINE void Array<Memory>::shrink()
{
    Memory::shrink();
}
template <typename Memory>
SKR_INLINE void Array<Memory>::resize(SizeType expect_size, const DataType& new_value)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        for (SizeType i = size(); i < expect_size; ++i)
        {
            new (data() + i) DataType(new_value);
        }
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::resize(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        memory::construct_stl_ub(data() + size(), expect_size - size());
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::resize_unsafe(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // destruct items if need
    if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::resize_default(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        memory::construct(data() + size(), expect_size - size());
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::resize_zeroed(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        std::memset(data() + size(), 0, (expect_size - size()) * sizeof(DataType));
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add(const DataType& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index(); i < size(); ++i)
    {
        new (data() + i) DataType(v);
    }
    return ref;
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add(DataType&& v)
{
    DataRef ref = add_unsafe();
    new (ref.ptr()) DataType(std::move(v));
    return ref;
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add_unique(const DataType& v)
{
    if (DataRef ref = find(v))
    {
        return ref;
    }
    else
    {
        return add(v);
    }
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add_unsafe(SizeType n)
{
    SizeType old_size = Memory::grow(n);
    return { data() + old_size, old_size };
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index(); i < size(); ++i)
    {
        new (data() + i) DataType();
    }
    return ref;
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::add_zeroed(SizeType n)
{
    DataRef ref = add_unsafe(n);
    std::memset(ref.ptr(), 0, n * sizeof(DataType));
    return ref;
}

// add at
template <typename Memory>
SKR_INLINE void Array<Memory>::add_at(SizeType idx, const DataType& v, SizeType n)
{
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (data() + idx + i) DataType(v);
    }
}
template <typename Memory>
SKR_INLINE void Array<Memory>::add_at(SizeType idx, DataType&& v)
{
    add_at_unsafe(idx);
    new (data() + idx) DataType(std::move(v));
}
template <typename Memory>
SKR_INLINE void Array<Memory>::add_at_unsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT((empty() && idx == 0) || is_valid_index(idx));
    auto move_n = size() - idx;
    add_unsafe(n);
    memory::move(data() + idx + n, data() + idx, move_n);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::add_at_default(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (data() + idx + i) DataType();
    }
}
template <typename Memory>
SKR_INLINE void Array<Memory>::add_at_zeroed(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    std::memset(data() + idx, 0, n * sizeof(DataType));
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::emplace(Args&&... args)
{
    DataRef ref = add_unsafe();
    new (ref.ptr()) DataType(std::forward<Args>(args)...);
    return ref;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE void Array<Memory>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (data() + index) DataType(std::forward<Args>(args)...);
}

// append
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::append(const Array& arr)
{
    if (arr.size())
    {
        DataRef ref = add_unsafe(arr.size());
        memory::copy(ref.ptr(), arr.data(), arr.size());
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::append(std::initializer_list<DataType> init_list)
{
    if (init_list.size())
    {
        DataRef ref = add_unsafe(init_list.size());
        memory::copy(ref.ptr(), init_list.begin(), init_list.size());
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::append(const U* p, SizeType n)
{
    if (n)
    {
        DataRef ref = add_unsafe(n);
        memory::copy(ref.ptr(), p, n);
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}

// append at
template <typename Memory>
SKR_INLINE void Array<Memory>::append_at(SizeType idx, const Array& arr)
{
    SKR_ASSERT(is_valid_index(idx));
    if (arr.size())
    {
        add_at_unsafe(idx, arr.size());
        memory::copy(data() + idx, arr.data(), arr.size());
    }
}
template <typename Memory>
SKR_INLINE void Array<Memory>::append_at(SizeType idx, std::initializer_list<DataType> init_list)
{
    SKR_ASSERT(is_valid_index(idx));
    if (init_list.size())
    {
        add_at_unsafe(idx, init_list.size());
        memory::copy(data() + idx, init_list.begin(), init_list.size());
    }
}
template <typename Memory>
template <typename U>
SKR_INLINE void Array<Memory>::append_at(SizeType idx, const U* p, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    if (n)
    {
        add_at_unsafe(idx, n);
        memory::copy(data() + idx, p, n);
    }
}

// operator append
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::operator+=(const DataType& v) { return add(v); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::operator+=(DataType&& v) { return add(std::move(v)); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::operator+=(std::initializer_list<DataType> init_list) { return append(init_list); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::operator+=(const Array<Memory>& arr) { return append(arr); }

// remove
template <typename Memory>
SKR_INLINE void Array<Memory>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index) && size() - index >= n);

    if (n)
    {
        // calc move size
        auto move_n = size() - index - n;

        // destruct remove items
        memory::destruct(data() + index, n);

        // move data
        if (move_n)
        {
            memory::move(data() + index, data() + size() - move_n, move_n);
        }

        // update size
        _set_size(size() - n);
    }
}
template <typename Memory>
SKR_INLINE void Array<Memory>::remove_at_swap(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index) && size() - index >= n);
    if (n)
    {
        // calc move size
        auto move_n = std::min(size() - index - n, n);

        // destruct remove items
        memory::destruct(data() + index, n);

        // move data
        if (move_n)
        {
            memory::move(data() + index, data() + size() - move_n, move_n);
        }

        // update size
        _set_size(size() - n);
    }
}
template <typename Memory>
template <typename U>
SKR_INLINE bool Array<Memory>::remove(const U& v)
{
    if (DataRef ref = find(v))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename U>
SKR_INLINE bool Array<Memory>::remove_swap(const U& v)
{
    if (DataRef ref = find(v))
    {
        remove_at_swap(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename U>
SKR_INLINE bool Array<Memory>::remove_last(const U& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename U>
SKR_INLINE bool Array<Memory>::remove_last_swap(const U& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at_swap(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::remove_all(const U& v)
{
    return remove_all_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::remove_all_swap(const U& v)
{
    return remove_all_if_swap([&v](const DataType& a) { return a == v; });
}

// remove by
template <typename Memory>
template <typename Pred>
SKR_INLINE bool Array<Memory>::remove_if(Pred&& pred)
{
    if (DataRef ref = find_if(std::forward<Pred>(pred)))
    {
        remove_at(ref.index);
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool Array<Memory>::remove_if_swap(Pred&& pred)
{
    if (DataRef ref = find_if(std::forward<Pred>(pred)))
    {
        remove_at_swap(ref.index);
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool Array<Memory>::remove_last_if(Pred&& pred)
{
    if (DataRef ref = find_last_if(std::forward<Pred>(pred)))
    {
        remove_at(ref.index);
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool Array<Memory>::remove_last_if_swap(Pred&& pred)
{
    if (DataRef ref = find_last_if(std::forward<Pred>(pred)))
    {
        remove_at_swap(ref.index);
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::remove_all_if(Pred&& pred)
{
    DataType* pos = algo::remove_all(begin(), end(), std::forward<Pred>(pred));
    SizeType  n   = end() - pos;
    _set_size(size() - n);
    return n;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::remove_all_if_swap(Pred&& pred)
{
    DataType* pos = algo::remove_all_swap(begin(), end(), pred);
    SizeType  n   = end() - pos;
    _set_size(size() - n);
    return n;
}

// erase
template <typename Memory>
typename Array<Memory>::StlIt Array<Memory>::erase(const StlIt& it)
{
    remove_at(it - begin());
    return it;
}
template <typename Memory>
typename Array<Memory>::CStlIt Array<Memory>::erase(const CStlIt& it)
{
    remove_at(it - begin());
    return it;
}
template <typename Memory>
typename Array<Memory>::StlIt Array<Memory>::erase_swap(const StlIt& it)
{
    remove_at_swap(it - begin());
    return it;
}
template <typename Memory>
typename Array<Memory>::CStlIt Array<Memory>::erase_swap(const CStlIt& it)
{
    remove_at_swap(it - begin());
    return it;
}

// modify
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::operator[](SizeType index)
{
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::operator[](SizeType index) const
{
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::at(SizeType index)
{
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::at(SizeType index) const
{
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::last(SizeType index)
{
    index = size() - index - 1;
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::last(SizeType index) const
{
    index = size() - index - 1;
    SKR_ASSERT(!empty() && is_valid_index(index));
    return *(data() + index);
}

// front/back
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::front()
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[0];
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::front() const
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[0];
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::back()
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[size() - 1];
}
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::back() const
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[size() - 1];
}
template <typename Memory>
SKR_INLINE void Array<Memory>::push_back(const DataType& v)
{
    add(v);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::push_back(DataType&& v)
{
    add(std::move(v));
}
template <typename Memory>
SKR_INLINE void Array<Memory>::pop_back()
{
    stack_pop();
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::pop_back_get()
{
    return stack_pop_get();
}

// find
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::find(const U& v)
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::find_last(const U& v)
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::CDataRef Array<Memory>::find(const U& v) const
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::CDataRef Array<Memory>::find_last(const U& v) const
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}

// find by
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::find_if(Pred&& pred)
{
    auto p_begin = data();
    auto p_end   = data() + size();

    for (; p_begin < p_end; ++p_begin)
    {
        if (pred(*p_begin))
        {
            return { p_begin, static_cast<SizeType>(p_begin - data()) };
        }
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::DataRef Array<Memory>::find_last_if(Pred&& pred)
{
    auto p_begin = data();
    auto p_end   = data() + size() - 1;

    for (; p_end >= p_begin; --p_end)
    {
        if (pred(*p_end))
        {
            return { p_end, static_cast<SizeType>(p_end - data()) };
        }
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::CDataRef Array<Memory>::find_if(Pred&& pred) const
{
    return const_cast<Array<Memory>*>(this)->find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::CDataRef Array<Memory>::find_last_if(Pred&& pred) const
{
    return const_cast<Array<Memory>*>(this)->find_last_if(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <typename U>
SKR_INLINE bool Array<Memory>::contains(const U& v) const { return (bool)find(v); }
template <typename Memory>
template <typename Pred>
SKR_INLINE bool Array<Memory>::contains_if(Pred&& pred) const
{
    return (bool)find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::count(const U& v) const
{
    SizeType count = 0;
    for (const DataType& data : *this)
    {
        if (data == v)
        {
            ++count;
        }
    }
    return count;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::count_if(Pred&& pred) const
{
    SizeType count = 0;
    for (const DataType& v : *this)
    {
        if (pred(v))
        {
            ++count;
        }
    }
    return count;
}

// sort
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::sort(Functor&& f)
{
    algo::intro_sort(begin(), end(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::sort_stable(Functor&& f)
{
    algo::merge_sort(begin(), end(), std::forward<Functor>(f));
}

// support heap
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::heap_top() { return *data(); }
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::heapify(Functor&& f)
{
    algo::heapify(data(), size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE bool Array<Memory>::is_heap(Functor&& f)
{
    return algo::is_heap(data(), size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::heap_push(DataType&& v, Functor&& f)
{
    DataRef ref = emplace(std::move(v));
    return algo::heap_sift_up(data(), (SizeType)0, ref.index(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Array<Memory>::SizeType Array<Memory>::heap_push(const DataType& v, Functor&& f)
{
    DataRef ref = add(v);
    return algo::heap_sift_up(data(), SizeType(0), ref.index(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::heap_pop(Functor&& f)
{
    remove_at_swap(0);
    algo::heap_sift_down(data(), (SizeType)0, size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Array<Memory>::DataType Array<Memory>::heap_pop_get(Functor&& f)
{
    DataType result = std::move(*data());
    heap_pop(std::forward<Functor>(f));
    return result;
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::heap_remove_at(SizeType index, Functor&& f)
{
    remove_at_swap(index);

    algo::heap_sift_down(data(), index, size(), std::forward<Functor>(f));
    algo::heap_sift_up(data(), (SizeType)0, std::min(index, size() - 1), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Array<Memory>::heap_sort(Functor&& f)
{
    algo::heap_sort(data(), size(), std::forward<Functor>(f));
}

// support stack
template <typename Memory>
SKR_INLINE void Array<Memory>::stack_pop(SizeType n)
{
    SKR_ASSERT(n > 0 && n <= size() && "pop size must be in [1, size()]");
    memory::destruct(data() + size() - n, n);
    _set_size(size() - n);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::stack_pop_unsafe(SizeType n)
{
    SKR_ASSERT(n > 0 && n <= size() && "pop size must be in [1, size()]");
    _set_size(size() - n);
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType Array<Memory>::stack_pop_get()
{
    SKR_ASSERT(size() > 0 && "pop an empty stack");
    DataType result = std::move(*(data() + size() - 1));
    stack_pop();
    return std::move(result);
}
template <typename Memory>
SKR_INLINE void Array<Memory>::stack_push(const DataType& v) { add(v); }
template <typename Memory>
SKR_INLINE void Array<Memory>::stack_push(DataType&& v) { add(std::move(v)); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::stack_top() { return *(data() + size() - 1); }
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::stack_top() const { return *(data() + size() - 1); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::DataType& Array<Memory>::stack_bottom() { return *data(); }
template <typename Memory>
SKR_INLINE const typename Array<Memory>::DataType& Array<Memory>::stack_bottom() const { return *data(); }

// cursor & iter
template <typename Memory>
SKR_INLINE typename Array<Memory>::Cursor Array<Memory>::cursor_begin()
{
    return Cursor::Begin(data(), size());
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::CCursor Array<Memory>::cursor_begin() const
{
    return CCursor::Begin(data(), size());
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::Cursor Array<Memory>::cursor_end()
{
    return Cursor::End(data(), size());
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::CCursor Array<Memory>::cursor_end() const
{
    return CCursor::End(data(), size());
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::Iter Array<Memory>::iter()
{
    return { Cursor::Begin(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::CIter Array<Memory>::iter() const
{
    return { CCursor::Begin(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::IterInv Array<Memory>::iter_inv()
{
    return { Cursor::End(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename Array<Memory>::CIterInv Array<Memory>::iter_inv() const
{
    return { CCursor::End(data(), size()) };
}

// support foreach
template <typename Memory>
SKR_INLINE typename Array<Memory>::StlIt Array<Memory>::begin() { return data(); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::StlIt Array<Memory>::end() { return data() + size(); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::CStlIt Array<Memory>::begin() const { return data(); }
template <typename Memory>
SKR_INLINE typename Array<Memory>::CStlIt Array<Memory>::end() const { return data() + size(); }

// syntax
template <typename Memory>
SKR_INLINE const Array<Memory>& Array<Memory>::readonly() const
{
    return *this;
}
} // namespace skr::container
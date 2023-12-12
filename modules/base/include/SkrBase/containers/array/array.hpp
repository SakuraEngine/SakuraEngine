#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"
#include "SkrBase/algo/remove.hpp"
#include "SkrBase/containers/array/array_def.hpp"

// Array def
namespace skr::container
{
template <typename T, typename Memory>
struct Array : private Memory {
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;
    using DataRef  = ArrayDataRef<T, SizeType>;
    using CDataRef = ArrayDataRef<const T, SizeType>;
    using It       = T*;
    using CIt      = const T*;

    // ctor & dtor
    Array(AllocatorCtorParam param = {}) noexcept;
    Array(SizeType size, AllocatorCtorParam param = {}) noexcept;
    Array(SizeType size, const T& v, AllocatorCtorParam param = {}) noexcept;
    Array(const T* p, SizeType n, AllocatorCtorParam param = {}) noexcept;
    Array(std::initializer_list<T> init_list, AllocatorCtorParam param = {}) noexcept;
    ~Array();

    // copy & move
    Array(const Array& other, AllocatorCtorParam param = {});
    Array(Array&& other) noexcept;

    // assign & move assign
    Array& operator=(const Array& rhs);
    Array& operator=(Array&& rhs) noexcept;

    // special assign
    void assign(const T* p, SizeType n);
    void assign(std::initializer_list<T> init_list);

    // compare
    bool operator==(const Array& rhs) const;
    bool operator!=(const Array& rhs) const;

    // getter
    SizeType      size() const;
    SizeType      capacity() const;
    SizeType      slack() const;
    bool          empty() const;
    T*            data();
    const T*      data() const;
    Memory&       memory();
    const Memory& memory() const;

    // validate
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const T* p) const;

    // memory op
    void clear();
    void release(SizeType reserve_capacity = 0);
    void reserve(SizeType expect_capacity);
    void shrink();
    void resize(SizeType expect_size, const T& new_value);
    void resize(SizeType expect_size);
    void resize_unsafe(SizeType expect_size);
    void resize_default(SizeType expect_size);
    void resize_zeroed(SizeType expect_size);

    // add
    DataRef add(const T& v, SizeType n = 1);
    DataRef add(T&& v);
    DataRef add_unique(const T& v);
    DataRef add_unsafe(SizeType n = 1);
    DataRef add_default(SizeType n = 1);
    DataRef add_zeroed(SizeType n = 1);

    // add at
    void add_at(SizeType idx, const T& v, SizeType n = 1);
    void add_at(SizeType idx, T&& v);
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
    DataRef append(std::initializer_list<T> init_list);
    DataRef append(const T* p, SizeType n);

    // append at
    void append_at(SizeType idx, const Array& arr);
    void append_at(SizeType idx, std::initializer_list<T> init_list);
    void append_at(SizeType idx, T* p, SizeType n);

    // operator append
    DataRef operator+=(const T& v);
    DataRef operator+=(T&& v);
    DataRef operator+=(std::initializer_list<T> init_list);
    DataRef operator+=(const Array& arr);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    void remove_at_swap(SizeType index, SizeType n = 1);
    template <typename TK>
    DataRef remove(const TK& v);
    template <typename TK>
    DataRef remove_swap(const TK& v);
    template <typename TK>
    DataRef remove_last(const TK& v);
    template <typename TK>
    DataRef remove_last_swap(const TK& v);
    template <typename TK>
    SizeType remove_all(const TK& v);
    template <typename TK>
    SizeType remove_all_swap(const TK& v);

    // remove if
    template <typename TP>
    DataRef remove_if(TP&& p);
    template <typename TP>
    DataRef remove_if_swap(TP&& p);
    template <typename TP>
    DataRef remove_last_if(TP&& p);
    template <typename TP>
    DataRef remove_last_if_swap(TP&& p);
    template <typename TP>
    SizeType remove_all_if(TP&& p);
    template <typename TP>
    SizeType remove_all_if_swap(TP&& p);

    // erase
    It  erase(const It& it);
    CIt erase(const CIt& it);
    It  erase_swap(const It& it);
    CIt erase_swap(const CIt& it);

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;
    T&       last(SizeType index = 0);
    const T& last(SizeType index = 0) const;

    // front/back
    T&       front();
    const T& front() const;
    T&       back();
    const T& back() const;
    void     push_back(const T& v);
    void     push_back(T&& v);
    void     pop_back();
    T&       pop_back_get();

    // find
    template <typename TK>
    DataRef find(const TK& v);
    template <typename TK>
    DataRef find_last(const TK& v);
    template <typename TK>
    CDataRef find(const TK& v) const;
    template <typename TK>
    CDataRef find_last(const TK& v) const;

    // find if
    template <typename TP>
    DataRef find_if(TP&& p);
    template <typename TP>
    DataRef find_last_if(TP&& p);
    template <typename TP>
    CDataRef find_if(TP&& p) const;
    template <typename TP>
    CDataRef find_last_if(TP&& p) const;

    // contain
    template <typename TK>
    bool contain(const TK& v) const;
    template <typename TP>
    bool contain_if(TP&& p) const;

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = {});
    template <typename TP = Less<T>>
    void sort_stable(TP&& p = {});

    // support heap
    T& heap_top();
    template <typename TP = Less<T>>
    void heapify(TP&& p = {});
    template <typename TP = Less<T>>
    bool is_heap(TP&& p = {});
    template <typename TP = Less<T>>
    SizeType heap_push(T&& v, TP&& p = {});
    template <typename TP = Less<T>>
    SizeType heap_push(const T& v, TP&& p = {});
    template <typename TP = Less<T>>
    void heap_pop(TP&& p = {});
    template <typename TP = Less<T>>
    T heap_pop_get(TP&& p = {});
    template <typename TP = Less<T>>
    void heap_remove_at(SizeType index, TP&& p = {});
    template <typename TP = Less<T>>
    void heap_sort(TP&& p = {});

    // support stack
    void     stack_pop(SizeType n = 1);
    void     stack_pop_unsafe(SizeType n = 1);
    T        stack_pop_get();
    void     stack_push(const T& v);
    void     stack_push(T&& v);
    T&       stack_top();
    const T& stack_top() const;
    T&       stack_bottom();
    const T& stack_bottom() const;

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::_realloc(SizeType expect_capacity)
{
    Memory::realloc(expect_capacity);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::_free()
{
    Memory::free();
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::_set_size(SizeType new_size)
{
    Memory::set_size(new_size);
}

// ctor & dtor
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(SizeType size, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize(size);
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(SizeType size, const T& v, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize(size, v);
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(const T* p, SizeType n, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(n);
    memory::copy(data(), p, n);
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(std::initializer_list<T> init_list, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(init_list.size());
    memory::copy(data(), init_list.begin(), init_list.size());
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::~Array()
{
    // handled in memory
}

// copy & move
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(const Array& other, AllocatorCtorParam param)
    : Memory(other, std::move(param))
{
    // handled in memory
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>::Array(Array&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>& Array<T, Memory>::operator=(const Array& rhs)
{
    if (this != &rhs)
    {
        clear();
        Memory::operator=(rhs);
    }

    return *this;
}
template <typename T, typename Memory>
SKR_INLINE Array<T, Memory>& Array<T, Memory>::operator=(Array&& rhs) noexcept
{
    if (this != &rhs)
    {
        clear();
        Memory::operator=(std::move(rhs));
    }

    return *this;
}

// special assign
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::assign(const T* p, SizeType n)
{
    // clear and resize
    clear();
    resize_unsafe(n);

    // copy items
    memory::copy(data(), p, n);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::assign(std::initializer_list<T> init_list)
{
    assign(init_list.begin(), init_list.size());
}

// compare
template <typename T, typename Memory>
SKR_INLINE bool Array<T, Memory>::operator==(const Array& rhs) const
{
    return size() == rhs.size() && memory::compare(data(), rhs.data(), size());
}
template <typename T, typename Memory>
SKR_INLINE bool Array<T, Memory>::operator!=(const Array& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::size() const
{
    return Memory::size();
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::slack() const
{
    return capacity() - size();
}
template <typename T, typename Memory>
SKR_INLINE bool Array<T, Memory>::empty() const
{
    return size() == 0;
}
template <typename T, typename Memory>
SKR_INLINE T* Array<T, Memory>::data()
{
    return Memory::data();
}
template <typename T, typename Memory>
SKR_INLINE const T* Array<T, Memory>::data() const
{
    return Memory::data();
}
template <typename T, typename Memory>
SKR_INLINE Memory& Array<T, Memory>::memory()
{
    return *this;
}
template <typename T, typename Memory>
SKR_INLINE const Memory& Array<T, Memory>::memory() const
{
    return *this;
}

// validate
template <typename T, typename Memory>
SKR_INLINE bool Array<T, Memory>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < size();
}
template <typename T, typename Memory>
SKR_INLINE bool Array<T, Memory>::is_valid_pointer(const T* p) const
{
    return p >= begin() && p < end();
}

// memory op
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::clear()
{
    memory::destruct(data(), size());
    _set_size(0);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::release(SizeType reserve_capacity)
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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::shrink()
{
    Memory::shrink();
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::resize(SizeType expect_size, const T& new_value)
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
            new (data() + i) T(new_value);
        }
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::resize(SizeType expect_size)
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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::resize_unsafe(SizeType expect_size)
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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::resize_default(SizeType expect_size)
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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::resize_zeroed(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        std::memset(data() + size(), 0, (expect_size - size()) * sizeof(T));
    }
    else if (expect_size < size())
    {
        memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add(const T& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index; i < size(); ++i)
    {
        new (data() + i) T(v);
    }
    return ref;
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add(T&& v)
{
    DataRef ref = add_unsafe();
    new (ref.data) T(std::move(v));
    return ref;
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add_unique(const T& v)
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
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add_unsafe(SizeType n)
{
    SizeType old_size = Memory::grow(n);
    return { data() + old_size, old_size };
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index; i < size(); ++i)
    {
        new (data() + i) T();
    }
    return ref;
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::add_zeroed(SizeType n)
{
    DataRef ref = add_unsafe(n);
    std::memset(ref.data, 0, n * sizeof(T));
    return ref;
}

// add at
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::add_at(SizeType idx, const T& v, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (data() + idx + i) T(v);
    }
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::add_at(SizeType idx, T&& v)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx);
    new (data() + idx) T(std::move(v));
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::add_at_unsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    auto move_n = size() - idx;
    add_unsafe(n);
    memory::move(data() + idx + n, data() + idx, move_n);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::add_at_default(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (data() + idx + i) T();
    }
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::add_at_zeroed(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    std::memset(data() + idx, 0, n * sizeof(T));
}

// emplace
template <typename T, typename Memory>
template <typename... Args>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::emplace(Args&&... args)
{
    DataRef ref = add_unsafe();
    new (ref.data) T(std::forward<Args>(args)...);
    return ref;
}
template <typename T, typename Memory>
template <typename... Args>
SKR_INLINE void Array<T, Memory>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (data() + index) T(std::forward<Args>(args)...);
}

// append
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::append(const Array& arr)
{
    if (arr.size())
    {
        DataRef ref = add_unsafe(arr.size());
        memory::copy(ref.data, arr.data(), arr.size());
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::append(std::initializer_list<T> init_list)
{
    if (init_list.size())
    {
        DataRef ref = add_unsafe(init_list.size());
        memory::copy(ref.data, init_list.begin(), init_list.size());
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::append(const T* p, SizeType n)
{
    if (n)
    {
        DataRef ref = add_unsafe(n);
        memory::copy(ref.data, p, n);
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}

// append at
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::append_at(SizeType idx, const Array& arr)
{
    SKR_ASSERT(is_valid_index(idx));
    if (arr.size())
    {
        add_at_unsafe(idx, arr.size());
        memory::copy(data() + idx, arr.data(), arr.size());
    }
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::append_at(SizeType idx, std::initializer_list<T> init_list)
{
    SKR_ASSERT(is_valid_index(idx));
    if (init_list.size())
    {
        add_at_unsafe(idx, init_list.size());
        memory::copy(data() + idx, init_list.begin(), init_list.size());
    }
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::append_at(SizeType idx, T* p, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    if (n)
    {
        add_at_unsafe(idx, n);
        memory::copy(data() + idx, p, n);
    }
}

// operator append
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::operator+=(const T& v) { return add(v); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::operator+=(T&& v) { return add(std::move(v)); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::operator+=(std::initializer_list<T> init_list) { return append(init_list); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::operator+=(const Array<T, Memory>& arr) { return append(arr); }

// remove
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= size());

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
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::remove_at_swap(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= size());
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
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove(const TK& v)
{
    if (DataRef ref = find(v))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_swap(const TK& v)
{
    if (DataRef ref = find(v))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_last(const TK& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_last_swap(const TK& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::remove_all(const TK& v)
{
    return remove_all_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::remove_all_swap(const TK& v)
{
    return remove_all_if_swap([&v](const T& a) { return a == v; });
}

// remove by
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_if(TP&& p)
{
    if (DataRef ref = find_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_if_swap(TP&& p)
{
    if (DataRef ref = find_if(std::forward<TP>(p)))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_last_if(TP&& p)
{
    if (DataRef ref = find_last_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::remove_last_if_swap(TP&& p)
{
    if (DataRef ref = find_last_if(std::forward<TP>(p)))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::remove_all_if(TP&& p)
{
    T*       pos = algo::remove_all(begin(), end(), std::forward<TP>(p));
    SizeType n   = end() - pos;
    _set_size(size() - n);
    return n;
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::remove_all_if_swap(TP&& p)
{
    T*       pos = algo::remove_all_swap(begin(), end(), p);
    SizeType n   = end() - pos;
    _set_size(size() - n);
    return n;
}

// erase
template <typename T, typename Memory>
typename Array<T, Memory>::It Array<T, Memory>::erase(const It& it)
{
    remove_at(it - begin());
    return it;
}
template <typename T, typename Memory>
typename Array<T, Memory>::CIt Array<T, Memory>::erase(const CIt& it)
{
    remove_at(it - begin());
    return it;
}
template <typename T, typename Memory>
typename Array<T, Memory>::It Array<T, Memory>::erase_swap(const It& it)
{
    remove_at_swap(it - begin());
    return it;
}
template <typename T, typename Memory>
typename Array<T, Memory>::CIt Array<T, Memory>::erase_swap(const CIt& it)
{
    remove_at_swap(it - begin());
    return it;
}

// modify
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::operator[](SizeType index)
{
    SKR_ASSERT(is_valid_index(index));
    return *(data() + index);
}
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::operator[](SizeType index) const
{
    SKR_ASSERT(is_valid_index(index));
    return *(data() + index);
}
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::last(SizeType index)
{
    index = size() - index - 1;
    SKR_ASSERT(is_valid_index(index));
    return *(data() + index);
}
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::last(SizeType index) const
{
    index = size() - index - 1;
    SKR_ASSERT(is_valid_index(index));
    return *(data() + index);
}

// front/back
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::front()
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[0];
}
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::front() const
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[0];
}
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::back()
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[size() - 1];
}
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::back() const
{
    SKR_ASSERT(size() > 0 && "visit an empty array");
    return data()[size() - 1];
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::push_back(const T& v)
{
    add(v);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::push_back(T&& v)
{
    add(std::move(v));
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::pop_back()
{
    stack_pop();
}
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::pop_back_get()
{
    return stack_pop_get();
}

// find
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::find(const TK& v)
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::find_last(const TK& v)
{
    return find_last_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::CDataRef Array<T, Memory>::find(const TK& v) const
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE typename Array<T, Memory>::CDataRef Array<T, Memory>::find_last(const TK& v) const
{
    return find_last_if([&v](const T& a) { return a == v; });
}

// find by
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::find_if(TP&& p)
{
    auto p_begin = data();
    auto p_end   = data() + size();

    for (; p_begin < p_end; ++p_begin)
    {
        if (p(*p_begin))
        {
            return { p_begin, static_cast<SizeType>(p_begin - data()) };
        }
    }
    return {};
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::DataRef Array<T, Memory>::find_last_if(TP&& p)
{
    auto p_begin = data();
    auto p_end   = data() + size() - 1;

    for (; p_end >= p_begin; --p_end)
    {
        if (p(*p_end))
        {
            return { p_end, static_cast<SizeType>(p_end - data()) };
        }
    }
    return {};
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::CDataRef Array<T, Memory>::find_if(TP&& p) const
{
    auto ref = const_cast<Array<T, Memory>*>(this)->find_if(std::forward<TP>(p));
    return { ref.data, ref.index };
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::CDataRef Array<T, Memory>::find_last_if(TP&& p) const
{
    auto ref = const_cast<Array<T, Memory>*>(this)->find_last_if(std::forward<TP>(p));
    return { ref.data, ref.index };
}

// contain
template <typename T, typename Memory>
template <typename TK>
SKR_INLINE bool Array<T, Memory>::contain(const TK& v) const { return (bool)find(v); }
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE bool Array<T, Memory>::contain_if(TP&& p) const
{
    return (bool)find_if(std::forward<TP>(p));
}

// sort
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::sort(TP&& p)
{
    algo::intro_sort(begin(), end(), std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::sort_stable(TP&& p)
{
    algo::merge_sort(begin(), end(), std::forward<TP>(p));
}

// support heap
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::heap_top() { return *data(); }
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::heapify(TP&& p)
{
    algo::heapify(data(), size(), std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE bool Array<T, Memory>::is_heap(TP&& p)
{
    return algo::is_heap(data(), size(), std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::heap_push(T&& v, TP&& p)
{
    DataRef ref = emplace(std::move(v));
    return algo::heap_sift_up(data(), (SizeType)0, ref.index, std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE typename Array<T, Memory>::SizeType Array<T, Memory>::heap_push(const T& v, TP&& p)
{
    DataRef ref = add(v);
    return algo::heap_sift_up(data(), SizeType(0), ref.index, std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::heap_pop(TP&& p)
{
    remove_at_swap(0);
    algo::heap_sift_down(data(), (SizeType)0, size(), std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE T Array<T, Memory>::heap_pop_get(TP&& p)
{
    T result = std::move(*data());
    heap_pop(std::forward<TP>(p));
    return result;
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::heap_remove_at(SizeType index, TP&& p)
{
    remove_at_swap(index);

    algo::heap_sift_down(data(), index, size(), std::forward<TP>(p));
    algo::heap_sift_up(data(), (SizeType)0, std::min(index, size() - 1), std::forward<TP>(p));
}
template <typename T, typename Memory>
template <typename TP>
SKR_INLINE void Array<T, Memory>::heap_sort(TP&& p)
{
    algo::heap_sort(data(), size(), std::forward<TP>(p));
}

// support stack
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::stack_pop(SizeType n)
{
    SKR_ASSERT(n > 0);
    SKR_ASSERT(n <= size());
    memory::destruct(data() + size() - n, n);
    _set_size(size() - n);
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::stack_pop_unsafe(SizeType n)
{
    SKR_ASSERT(n > 0);
    SKR_ASSERT(n <= size());
    _set_size(size() - n);
}
template <typename T, typename Memory>
SKR_INLINE T Array<T, Memory>::stack_pop_get()
{
    T result = std::move(*(data() + size() - 1));
    stack_pop();
    return result;
}
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::stack_push(const T& v) { add(v); }
template <typename T, typename Memory>
SKR_INLINE void Array<T, Memory>::stack_push(T&& v) { add(std::move(v)); }
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::stack_top() { return *(data() + size() - 1); }
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::stack_top() const { return *(data() + size() - 1); }
template <typename T, typename Memory>
SKR_INLINE T& Array<T, Memory>::stack_bottom() { return *data(); }
template <typename T, typename Memory>
SKR_INLINE const T& Array<T, Memory>::stack_bottom() const { return *data(); }

// support foreach
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::It Array<T, Memory>::begin() { return data(); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::It Array<T, Memory>::end() { return data() + size(); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::CIt Array<T, Memory>::begin() const { return data(); }
template <typename T, typename Memory>
SKR_INLINE typename Array<T, Memory>::CIt Array<T, Memory>::end() const { return data() + size(); }
} // namespace skr::container
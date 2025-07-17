#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"
#include "SkrBase/algo/remove.hpp"
#include "SkrBase/containers/vector/vector_def.hpp"
#include "SkrBase/containers/vector/vector_iterator.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"
#include "SkrBase/containers/misc/span.hpp"
#include "SkrBase/template/concepts.hpp"

// Vector def
// TODO. 针对 NoneCopyable 成员 (典型的如 UniquePtr) 的支持，抑制默认实现 copy 的报错
namespace skr::container
{
template <typename Memory>
struct Vector : protected Memory {
    // from memory
    using typename Memory::DataType;
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;

    // data ref
    using DataRef  = VectorDataRef<DataType, SizeType, false>;
    using CDataRef = VectorDataRef<DataType, SizeType, true>;

    // cursor & iterator
    using Cursor   = VectorCursor<Vector, false>;
    using CCursor  = VectorCursor<Vector, true>;
    using Iter     = VectorIter<Vector, false>;
    using CIter    = VectorIter<Vector, true>;
    using IterInv  = VectorIterInv<Vector, false>;
    using CIterInv = VectorIterInv<Vector, true>;

    // stl iterator
    using StlIt  = DataType*;
    using CStlIt = const DataType*;

    // ctor & dtor
    Vector(AllocatorCtorParam param = {}) noexcept;
    Vector(SizeType size, AllocatorCtorParam param = {}) noexcept;
    Vector(SizeType size, const DataType& v, AllocatorCtorParam param = {}) noexcept;
    Vector(const DataType* p, SizeType n, AllocatorCtorParam param = {}) noexcept;
    Vector(std::initializer_list<DataType> init_list, AllocatorCtorParam param = {}) noexcept;
    Vector(Span<DataType, SizeType> span, AllocatorCtorParam param = {}) noexcept;
    ~Vector();

    // copy & move
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;

    // assign & move assign
    Vector& operator=(const Vector& rhs);
    Vector& operator=(Vector&& rhs) noexcept;

    // special assign
    void assign(const DataType* p, SizeType n);
    void assign(std::initializer_list<DataType> init_list);
    template <EachAbleContainer U>
    void assign(U&& container);

    // compare
    bool operator==(const Vector& rhs) const;
    bool operator!=(const Vector& rhs) const;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    bool            is_empty() const;
    DataType*       data();
    const DataType* data() const;
    Memory&         memory();
    const Memory&   memory() const;

    // validate
    bool is_valid_index(SizeType idx) const;

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
    requires(std::is_constructible_v<typename Memory::DataType, Args...>)
    DataRef emplace(Args&&... args);
    template <typename... Args>
    requires(std::is_constructible_v<typename Memory::DataType, Args...>)
    void emplace_at(SizeType index, Args&&... args);

    // append
    DataRef append(const Vector<Memory>& vec);
    DataRef append(std::initializer_list<DataType> init_list);
    template <EachAbleContainer U>
    DataRef append(U&& container);
    template <typename U = DataType>
    DataRef append(const U* p, SizeType n);

    // append at
    void append_at(SizeType idx, const Vector& arr);
    void append_at(SizeType idx, std::initializer_list<DataType> init_list);
    template <EachAbleContainer U>
    void append_at(SizeType idx, U&& container);
    template <typename U = DataType>
    void append_at(SizeType idx, const U* p, SizeType n);

    // operator append
    DataRef operator+=(const DataType& v);
    DataRef operator+=(DataType&& v);
    DataRef operator+=(const Vector<Memory>& vec);
    DataRef operator+=(std::initializer_list<DataType> init_list);
    template <EachAbleContainer U = std::initializer_list<SizeType>>
    DataRef operator+=(U&& container);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    void remove_at_swap(SizeType index, SizeType n = 1);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    bool remove(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    bool remove_swap(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    bool remove_last(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    bool remove_last_swap(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    SizeType remove_all(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    SizeType remove_all_swap(const U& v);

    // remove if
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    bool remove_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    bool remove_if_swap(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    bool remove_last_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    bool remove_last_if_swap(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    SizeType remove_all_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    SizeType remove_all_if_swap(Pred&& pred);

    // modify
    DataType&       operator[](SizeType index);
    const DataType& operator[](SizeType index) const;
    DataType&       at(SizeType index);
    const DataType& at(SizeType index) const;
    DataType&       at_last(SizeType index = 0);
    const DataType& at_last(SizeType index = 0) const;

    // front/back
    DataType&       front();
    const DataType& front() const;
    DataType&       back();
    const DataType& back() const;
    void            push_back(const DataType& v);
    void            push_back(DataType&& v);
    void            pop_back();
    DataType        pop_back_get();

    // find
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    DataRef find(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    DataRef find_last(const U& v);
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    CDataRef find(const U& v) const;
    template <typename U = DataType>
    requires(concepts::HasEq<typename Memory::DataType, U>)
    CDataRef find_last(const U& v) const;

    // find if
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <typename U = DataType>
    bool contains(const U& v) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
    bool contains_if(Pred&& pred) const;
    template <typename U = DataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
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
    bool is_heap(Functor&& f = {}) const;
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
    auto     range();
    auto     range() const;
    auto     range_inv();
    auto     range_inv() const;

    // stl-style iterator
    StlIt  begin();
    StlIt  end();
    CStlIt begin() const;
    CStlIt end() const;

    // erase
    StlIt  erase(const StlIt& it);
    CStlIt erase(const CStlIt& it);
    void   erase(const DataRef& ref);
    void   erase(const CDataRef& ref);
    StlIt  erase_swap(const StlIt& it);
    CStlIt erase_swap(const CStlIt& it);
    void   erase_swap(const DataRef& ref);
    void   erase_swap(const CDataRef& ref);

    // syntax
    const Vector&                  readonly() const;
    Span<DataType, SizeType>       span();
    Span<const DataType, SizeType> span() const;

private:
    // helper
    void _realloc(SizeType expect_capacity);
    void _free();
    void _set_size(SizeType new_size);
};
} // namespace skr::container

// Vector impl
namespace skr::container
{
// helper
template <typename Memory>
SKR_INLINE void Vector<Memory>::_realloc(SizeType expect_capacity)
{
    Memory::realloc(expect_capacity);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::_set_size(SizeType new_size)
{
    Memory::set_size(new_size);
}

// ctor & dtor
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(SizeType size, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_default(size);
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(SizeType size, const DataType& v, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize(size, v);
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(const DataType* p, SizeType n, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(n);
    ::skr::memory::copy(data(), p, n);
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(std::initializer_list<DataType> init_list, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(init_list.size());
    ::skr::memory::copy(data(), init_list.begin(), init_list.size());
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(Span<DataType, SizeType> span, AllocatorCtorParam param) noexcept
    : Memory(std::move(param))
{
    resize_unsafe(span.size());
    ::skr::memory::copy(data(), span.data(), span.size());
}
template <typename Memory>
SKR_INLINE Vector<Memory>::~Vector()
{
    // handled in memory
}

// copy & move
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(const Vector& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE Vector<Memory>::Vector(Vector&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
SKR_INLINE Vector<Memory>& Vector<Memory>::operator=(const Vector& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE Vector<Memory>& Vector<Memory>::operator=(Vector&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// special assign
template <typename Memory>
SKR_INLINE void Vector<Memory>::assign(const DataType* p, SizeType n)
{
    // clear and resize
    clear();
    resize_unsafe(n);

    // copy items
    ::skr::memory::copy(data(), p, n);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::assign(std::initializer_list<DataType> init_list)
{
    assign(init_list.begin(), init_list.size());
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE void Vector<Memory>::assign(U&& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;

    clear();

    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        resize_unsafe(n);
        ::skr::memory::copy(data(), p, n);
    }
    else if constexpr (Traits::is_iterable && Traits::has_size)
    {
        auto n     = Traits::size(std::forward<U>(container));
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        reserve(n);
        for (; begin != end; ++begin)
        {
            emplace(*begin);
        }
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        for (; begin != end; ++begin)
        {
            emplace(*begin);
        }
    }
}

// compare
template <typename Memory>
SKR_INLINE bool Vector<Memory>::operator==(const Vector& rhs) const
{
    return size() == rhs.size() && ::skr::memory::compare(data(), rhs.data(), size());
}
template <typename Memory>
SKR_INLINE bool Vector<Memory>::operator!=(const Vector& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename Memory>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::slack() const
{
    return capacity() - size();
}
template <typename Memory>
SKR_INLINE bool Vector<Memory>::is_empty() const
{
    return size() == 0;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType* Vector<Memory>::data()
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType* Vector<Memory>::data() const
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE Memory& Vector<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& Vector<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool Vector<Memory>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < size();
}

// memory op
template <typename Memory>
SKR_INLINE void Vector<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::release(SizeType reserve_capacity)
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
SKR_INLINE void Vector<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::shrink()
{
    Memory::shrink();
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::resize(SizeType expect_size, const DataType& new_value)
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
        ::skr::memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::resize_unsafe(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // destruct items if need
    if (expect_size < size())
    {
        ::skr::memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::resize_default(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        ::skr::memory::construct(data() + size(), expect_size - size());
    }
    else if (expect_size < size())
    {
        ::skr::memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::resize_zeroed(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        std::memset(reinterpret_cast<void*>(data() + size()), 0, (expect_size - size()) * sizeof(DataType));
    }
    else if (expect_size < size())
    {
        ::skr::memory::destruct(data() + expect_size, size() - expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add(const DataType& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index(); i < size(); ++i)
    {
        new (data() + i) DataType(v);
    }
    return ref;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add(DataType&& v)
{
    DataRef ref = add_unsafe();
    new (ref.ptr()) DataType(std::move(v));
    return ref;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add_unique(const DataType& v)
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
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add_unsafe(SizeType n)
{
    SizeType old_size = Memory::grow(n);
    return { data() + old_size, old_size };
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    ::skr::memory::construct(ref.ptr(), n);
    return ref;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::add_zeroed(SizeType n)
{
    DataRef ref = add_unsafe(n);
    std::memset(ref.ptr(), 0, n * sizeof(DataType));
    return ref;
}

// add at
template <typename Memory>
SKR_INLINE void Vector<Memory>::add_at(SizeType idx, const DataType& v, SizeType n)
{
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (data() + idx + i) DataType(v);
    }
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::add_at(SizeType idx, DataType&& v)
{
    add_at_unsafe(idx);
    new (data() + idx) DataType(std::move(v));
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::add_at_unsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT((is_empty() && idx == 0) || is_valid_index(idx));
    auto move_n = size() - idx;
    add_unsafe(n);
    ::skr::memory::move(data() + idx + n, data() + idx, move_n);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::add_at_default(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    ::skr::memory::construct(data() + idx, n);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::add_at_zeroed(SizeType idx, SizeType n)
{
    add_at_unsafe(idx, n);
    std::memset(data() + idx, 0, n * sizeof(DataType));
}

// emplace
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::DataType, Args...>)
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::emplace(Args&&... args)
{
    DataRef ref = add_unsafe();
    new (ref.ptr()) DataType(std::forward<Args>(args)...);
    return ref;
}
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::DataType, Args...>)
SKR_INLINE void Vector<Memory>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (data() + index) DataType(std::forward<Args>(args)...);
}

// append
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::append(const Vector<Memory>& vec)
{
    return append(vec.data(), vec.size());
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::append(std::initializer_list<DataType> init_list)
{
    return append(init_list.begin(), init_list.size());
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::append(U&& container)
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
            emplace(*begin);
        }
        return { data() + old_size, old_size };
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin    = Traits::begin(std::forward<U>(container));
        auto end      = Traits::end(std::forward<U>(container));
        auto old_size = size();
        for (; begin != end; ++begin)
        {
            emplace(*begin);
        }
        return { data() + old_size, old_size };
    }
    return {};
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::append(const U* p, SizeType n)
{
    if (n)
    {
        DataRef ref = add_unsafe(n);
        ::skr::memory::copy(ref.ptr(), p, n);
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}

// append at
template <typename Memory>
SKR_INLINE void Vector<Memory>::append_at(SizeType idx, const Vector& arr)
{
    if (arr.size())
    {
        add_at_unsafe(idx, arr.size());
        ::skr::memory::copy(data() + idx, arr.data(), arr.size());
    }
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::append_at(SizeType idx, std::initializer_list<DataType> init_list)
{
    if (init_list.size())
    {
        add_at_unsafe(idx, init_list.size());
        ::skr::memory::copy(data() + idx, init_list.begin(), init_list.size());
    }
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE void Vector<Memory>::append_at(SizeType idx, U&& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        if (n)
        {
            add_at_unsafe(idx, n);
            ::skr::memory::copy(data() + idx, p, n);
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
            new (data() + idx) DataType(*begin);
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
            emplace_at(idx, *begin);
            ++idx;
        }
    }
}
template <typename Memory>
template <typename U>
SKR_INLINE void Vector<Memory>::append_at(SizeType idx, const U* p, SizeType n)
{
    if (n)
    {
        add_at_unsafe(idx, n);
        ::skr::memory::copy(data() + idx, p, n);
    }
}

// operator append
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::operator+=(const DataType& v)
{
    return add(v);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::operator+=(DataType&& v)
{
    return add(std::move(v));
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::operator+=(const Vector<Memory>& vec)
{
    return append(vec);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::operator+=(std::initializer_list<DataType> init_list)
{
    return append(init_list);
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::operator+=(U&& container)
{
    return append(std::forward<U>(container));
}

// remove
template <typename Memory>
SKR_INLINE void Vector<Memory>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index) && size() - index >= n);

    if (n)
    {
        // calc move size
        auto move_n = size() - index - n;

        // destruct remove items
        ::skr::memory::destruct(data() + index, n);

        // move data
        if (move_n)
        {
            ::skr::memory::move(data() + index, data() + size() - move_n, move_n);
        }

        // update size
        _set_size(size() - n);
    }
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::remove_at_swap(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index) && size() - index >= n);
    if (n)
    {
        // calc move size
        auto move_n = std::min(size() - index - n, n);

        // destruct remove items
        ::skr::memory::destruct(data() + index, n);

        // move data
        if (move_n)
        {
            ::skr::memory::move(data() + index, data() + size() - move_n, move_n);
        }

        // update size
        _set_size(size() - n);
    }
}
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE bool Vector<Memory>::remove(const U& v)
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
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE bool Vector<Memory>::remove_swap(const U& v)
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
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE bool Vector<Memory>::remove_last(const U& v)
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
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE bool Vector<Memory>::remove_last_swap(const U& v)
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
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::remove_all(const U& v)
{
    return remove_all_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::remove_all_swap(const U& v)
{
    return remove_all_if_swap([&v](const DataType& a) { return a == v; });
}

// remove by
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE bool Vector<Memory>::remove_if(Pred&& pred)
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE bool Vector<Memory>::remove_if_swap(Pred&& pred)
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE bool Vector<Memory>::remove_last_if(Pred&& pred)
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE bool Vector<Memory>::remove_last_if_swap(Pred&& pred)
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::remove_all_if(Pred&& pred)
{
    DataType* pos = algo::remove_all(begin(), end(), std::forward<Pred>(pred));
    SizeType  n   = end() - pos;
    _set_size(size() - n);
    return n;
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::remove_all_if_swap(Pred&& pred)
{
    DataType* pos = algo::remove_all_swap(begin(), end(), pred);
    SizeType  n   = end() - pos;
    _set_size(size() - n);
    return n;
}

// modify
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::operator[](SizeType index)
{
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::operator[](SizeType index) const
{
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::at(SizeType index)
{
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::at(SizeType index) const
{
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::at_last(SizeType index)
{
    index = size() - index - 1;
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::at_last(SizeType index) const
{
    index = size() - index - 1;
    SKR_ASSERT(!is_empty() && is_valid_index(index));
    return *(data() + index);
}

// front/back
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::front()
{
    SKR_ASSERT(size() > 0 && "visit an empty vector");
    return data()[0];
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::front() const
{
    SKR_ASSERT(size() > 0 && "visit an empty vector");
    return data()[0];
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::back()
{
    SKR_ASSERT(size() > 0 && "visit an empty vector");
    return data()[size() - 1];
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::back() const
{
    SKR_ASSERT(size() > 0 && "visit an empty vector");
    return data()[size() - 1];
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::push_back(const DataType& v)
{
    add(v);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::push_back(DataType&& v)
{
    add(std::move(v));
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::pop_back()
{
    stack_pop();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType Vector<Memory>::pop_back_get()
{
    return std::move(stack_pop_get());
}

// find
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::find(const U& v)
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::find_last(const U& v)
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::CDataRef Vector<Memory>::find(const U& v) const
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
requires(concepts::HasEq<typename Memory::DataType, U>)
SKR_INLINE typename Vector<Memory>::CDataRef Vector<Memory>::find_last(const U& v) const
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}

// find by
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::find_if(Pred&& pred)
{
    if (!is_empty())
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
    }
    return {};
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::DataRef Vector<Memory>::find_last_if(Pred&& pred)
{
    if (!is_empty())
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
    }
    return {};
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::CDataRef Vector<Memory>::find_if(Pred&& pred) const
{
    return const_cast<Vector<Memory>*>(this)->find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::CDataRef Vector<Memory>::find_last_if(Pred&& pred) const
{
    return const_cast<Vector<Memory>*>(this)->find_last_if(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <typename U>
SKR_INLINE bool Vector<Memory>::contains(const U& v) const
{
    return (bool)find(v);
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE bool Vector<Memory>::contains_if(Pred&& pred) const
{
    return (bool)find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename U>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::count(const U& v) const
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::DataType&>)
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::count_if(Pred&& pred) const
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
SKR_INLINE void Vector<Memory>::sort(Functor&& f)
{
    algo::intro_sort(begin(), end(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Vector<Memory>::sort_stable(Functor&& f)
{
    algo::merge_sort(begin(), end(), std::forward<Functor>(f));
}

// support heap
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::heap_top()
{
    return *data();
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Vector<Memory>::heapify(Functor&& f)
{
    algo::heapify(data(), size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE bool Vector<Memory>::is_heap(Functor&& f) const
{
    return algo::is_heap(data(), size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::heap_push(DataType&& v, Functor&& f)
{
    DataRef ref = emplace(std::move(v));
    return algo::heap_sift_up(data(), (SizeType)0, ref.index(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Vector<Memory>::SizeType Vector<Memory>::heap_push(const DataType& v, Functor&& f)
{
    DataRef ref = add(v);
    return algo::heap_sift_up(data(), SizeType(0), ref.index(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Vector<Memory>::heap_pop(Functor&& f)
{
    remove_at_swap(0);
    algo::heap_sift_down(data(), (SizeType)0, size(), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE typename Vector<Memory>::DataType Vector<Memory>::heap_pop_get(Functor&& f)
{
    DataType result = std::move(*data());
    heap_pop(std::forward<Functor>(f));
    return result;
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Vector<Memory>::heap_remove_at(SizeType index, Functor&& f)
{
    remove_at_swap(index);

    algo::heap_sift_down(data(), index, size(), std::forward<Functor>(f));
    algo::heap_sift_up(data(), (SizeType)0, std::min(index, size() - 1), std::forward<Functor>(f));
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void Vector<Memory>::heap_sort(Functor&& f)
{
    algo::heap_sort(data(), size(), std::forward<Functor>(f));
}

// support stack
template <typename Memory>
SKR_INLINE void Vector<Memory>::stack_pop(SizeType n)
{
    SKR_ASSERT(n > 0 && n <= size() && "pop size must be in [1, size()]");
    ::skr::memory::destruct(data() + size() - n, n);
    _set_size(size() - n);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::stack_pop_unsafe(SizeType n)
{
    SKR_ASSERT(n > 0 && n <= size() && "pop size must be in [1, size()]");
    _set_size(size() - n);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType Vector<Memory>::stack_pop_get()
{
    SKR_ASSERT(size() > 0 && "pop an empty stack");
    DataType result = std::move(*(data() + size() - 1));
    stack_pop();
    return std::move(result);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::stack_push(const DataType& v)
{
    add(v);
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::stack_push(DataType&& v)
{
    add(std::move(v));
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::stack_top()
{
    return *(data() + size() - 1);
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::stack_top() const
{
    return *(data() + size() - 1);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::DataType& Vector<Memory>::stack_bottom()
{
    return *data();
}
template <typename Memory>
SKR_INLINE const typename Vector<Memory>::DataType& Vector<Memory>::stack_bottom() const
{
    return *data();
}

// cursor & iter
template <typename Memory>
SKR_INLINE typename Vector<Memory>::Cursor Vector<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CCursor Vector<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::Cursor Vector<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CCursor Vector<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::Iter Vector<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CIter Vector<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::IterInv Vector<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CIterInv Vector<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto Vector<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto Vector<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto Vector<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto Vector<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename Vector<Memory>::StlIt Vector<Memory>::begin()
{
    return data();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::StlIt Vector<Memory>::end()
{
    return data() + size();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CStlIt Vector<Memory>::begin() const
{
    return data();
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CStlIt Vector<Memory>::end() const
{
    return data() + size();
}

// erase
template <typename Memory>
SKR_INLINE typename Vector<Memory>::StlIt Vector<Memory>::erase(const StlIt& it)
{
    remove_at(it - begin());
    return it;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CStlIt Vector<Memory>::erase(const CStlIt& it)
{
    remove_at(it - begin());
    return it;
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::erase(const DataRef& ref)
{
    remove_at(ref.index());
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::erase(const CDataRef& ref)
{
    remove_at(ref.index());
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::StlIt Vector<Memory>::erase_swap(const StlIt& it)
{
    remove_at_swap(it - begin());
    return it;
}
template <typename Memory>
SKR_INLINE typename Vector<Memory>::CStlIt Vector<Memory>::erase_swap(const CStlIt& it)
{
    remove_at_swap(it - begin());
    return it;
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::erase_swap(const DataRef& ref)
{
    remove_at_swap(ref.index());
}
template <typename Memory>
SKR_INLINE void Vector<Memory>::erase_swap(const CDataRef& ref)
{
    remove_at_swap(ref.index());
}

// syntax
template <typename Memory>
SKR_INLINE const Vector<Memory>& Vector<Memory>::readonly() const
{
    return *this;
}
template <typename Memory>
SKR_INLINE Span<typename Vector<Memory>::DataType, typename Vector<Memory>::SizeType> Vector<Memory>::span()
{
    return { data(), size() };
}
template <typename Memory>
SKR_INLINE Span<const typename Vector<Memory>::DataType, typename Vector<Memory>::SizeType> Vector<Memory>::span() const
{
    return { data(), size() };
}
} // namespace skr::container

// container traits
namespace skr::container
{
template <typename Memory>
struct ContainerTraits<Vector<Memory>> {
    constexpr static bool is_linear_memory = true; // data(), size()
    constexpr static bool has_size         = true; // size()
    constexpr static bool is_iterable      = true; // begin(), end()
    constexpr static bool is_reservable    = true; // reserve()

    using ElementType = typename Memory::DataType;
    using SizeType    = typename Memory::SizeType;

    inline static const typename Vector<Memory>::DataType* data(const Vector<Memory>& vec) { return vec.data(); }
    inline static typename Vector<Memory>::DataType*       data(Vector<Memory>& vec) { return vec.data(); }
    inline static size_t                                   size(const Vector<Memory>& vec) { return vec.size(); }

    inline static auto begin(const Vector<Memory>& vec) noexcept { return vec.begin(); }
    inline static auto end(const Vector<Memory>& vec) noexcept { return vec.end(); }
    inline static auto begin(Vector<Memory>& vec) noexcept { return vec.begin(); }
    inline static auto end(Vector<Memory>& vec) noexcept { return vec.end(); }

    inline static void reserve(Vector<Memory>& vec, size_t n) { vec.reserve(n); }
};
} // namespace skr::container
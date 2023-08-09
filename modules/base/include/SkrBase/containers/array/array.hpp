#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"
#include "SkrBase/algo/remove.hpp"
#include "SkrBase/algo/find.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/containers/array/array_def.hpp"

// Array def
namespace skr
{
// TODO. 可能可以继承出来一个 ArrayMap/ArraySet
template <typename T, typename Alloc>
class Array
{
public:
    using SizeType = typename Alloc::SizeType;
    using DataRef  = ArrayDataRef<T, SizeType>;
    using CDataRef = ArrayDataRef<const T, SizeType>;

    // ctor & dtor
    Array(Alloc alloc = {});
    Array(SizeType size, Alloc alloc = {});
    Array(SizeType size, const T& v, Alloc alloc = {});
    Array(const T* p, SizeType n, Alloc alloc = {});
    Array(std::initializer_list<T> init_list, Alloc alloc = {});
    ~Array();

    // copy & move
    Array(const Array& other, Alloc alloc = {});
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
    SizeType     size() const;
    SizeType     capacity() const;
    SizeType     slack() const;
    bool         empty();
    T*           data();
    const T*     data() const;
    Alloc&       allocator();
    const Alloc& allocator() const;

    // validate
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const T* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    void resize(SizeType size, const T& new_value);
    void resize_unsafe(SizeType size);
    void resize_default(SizeType size);
    void resize_zeroed(SizeType size);

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
    // TODO. emplace unique

    // append
    DataRef append(const Array& arr);
    DataRef append(std::initializer_list<T> init_list);
    DataRef append(T* p, SizeType n);

    // append at
    void append_at(SizeType idx, const Array& arr);
    void append_at(SizeType idx, std::initializer_list<T> init_list);
    void append_at(SizeType idx, T* p, SizeType n);

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

    // TODO. move item

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;
    T&       last(SizeType index);
    const T& last(SizeType index) const;

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

    // TODO. binary add & remove & find

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
    void     pop(SizeType n = 1);
    void     pop_unsafe(SizeType n = 1);
    T        pop_get();
    void     push(const T& v);
    void     push(T&& v);
    T&       top();
    const T& top() const;
    T&       bottom();
    const T& bottom() const;

    // support foreach
    T*       begin();
    T*       end();
    const T* begin() const;
    const T* end() const;

private:
    // helper
    void _resize_memory(SizeType new_capacity);
    void _grow(SizeType n);

private:
    T*       _data;
    SizeType _size;
    SizeType _capacity;
    Alloc    _alloc;
};
} // namespace skr

// Array impl
namespace skr
{
// helper
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::_resize_memory(SizeType new_capacity)
{
    if (new_capacity)
    {
        // realloc
        _data     = _alloc.resize_container(_data, _size, _capacity, new_capacity);
        _size     = std::min(_size, _capacity);
        _capacity = new_capacity;
    }
    else if (_data)
    {
        // free
        memory::destruct(_data, _size);
        _alloc.free(_data);
        _data     = nullptr;
        _size     = 0;
        _capacity = 0;
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::_grow(SizeType n)
{
    auto new_size = _size + n;

    // grow memory
    if (new_size > _capacity)
    {
        auto new_capacity = _alloc.getGrow(new_size, _capacity);
        _data             = _alloc.resizeContainer(_data, _size, _capacity, new_capacity);
        _capacity         = new_capacity;
    }

    // update size
    _size = new_size;
}

// ctor & dtor
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(SizeType size, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resize_default(size);
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(SizeType size, const T& v, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resize(size, v);
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(const T* p, SizeType n, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resize_unsafe(n);
    memory::copy(_data, p, n);
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(std::initializer_list<T> init_list, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resize_unsafe(init_list.size());
    memory::copy(_data, init_list.begin(), init_list.size());
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::~Array()
{
    release();
}

// copy & move
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(const Array& other, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resize_unsafe(other.size());
    memory::copy(_data, other.data(), other.size());
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(Array&& other) noexcept
    : _data(other._data)
    , _size(other._size)
    , _capacity(other._capacity)
    , _alloc(std::move(other._alloc))
{
    other._data     = nullptr;
    other._size     = 0;
    other._capacity = 0;
}

// assign & move assign
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>& Array<T, Alloc>::operator=(const Array& rhs)
{
    if (this != &rhs)
    {
        // clear and resize
        clear();
        resize_unsafe(rhs._size);

        // copy items
        memory::copy(_data, rhs._data, rhs._size);
    }
    return *this;
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>& Array<T, Alloc>::operator=(Array&& rhs) noexcept
{
    if (this != &rhs)
    {
        // release
        release();

        // copy data
        _data     = rhs._data;
        _size     = rhs._size;
        _capacity = rhs._capacity;
        _alloc    = std::move(rhs._alloc);

        // invalidate rhs
        rhs._data     = nullptr;
        rhs._size     = 0;
        rhs._capacity = 0;
    }
    return *this;
}

// special assign
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::assign(const T* p, SizeType n)
{
    // clear and resize
    clear();
    resize_unsafe(n);

    // copy items
    memory::copy(_data, p, n);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::assign(std::initializer_list<T> init_list)
{
    assign(init_list.begin(), init_list.size());
}

// compare
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::operator==(const Array& rhs) const
{
    return _size == rhs._size && memory::compare(_data, rhs._data, _size);
}
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::operator!=(const Array& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::size() const { return _size; }
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::capacity() const { return _capacity; }
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::slack() const { return _capacity - _size; }
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::empty() { return _size == 0; }
template <typename T, typename Alloc>
SKR_INLINE T* Array<T, Alloc>::data() { return _data; }
template <typename T, typename Alloc>
SKR_INLINE const T* Array<T, Alloc>::data() const { return _data; }
template <typename T, typename Alloc>
SKR_INLINE Alloc& Array<T, Alloc>::allocator() { return _alloc; }
template <typename T, typename Alloc>
SKR_INLINE const Alloc& Array<T, Alloc>::allocator() const { return _alloc; }

// validate
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::is_valid_index(SizeType idx) const { return idx >= 0 && idx < _size; }
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::is_valid_pointer(const T* p) const { return p >= begin() && p < end(); }

// memory op
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::clear()
{
    if (_size)
    {
        memory::destruct(_data, _size);
        _size = 0;
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::release(SizeType capacity)
{
    clear();
    _resize_memory(capacity);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::reserve(SizeType capacity)
{
    if (capacity > _capacity)
    {
        _resize_memory(capacity);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::shrink()
{
    auto new_capacity = _alloc.get_shrink(_size, _capacity);
    _resize_memory(new_capacity);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resize(SizeType size, const T& new_value)
{
    // need grow
    if (size > _capacity)
    {
        _resize_memory(size);
    }

    // add or remove
    if (size > _size)
    {
        add(new_value, size - _size);
    }
    else if (size < _size)
    {
        remove_at(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resize_unsafe(SizeType size)
{
    // need grow
    if (size > _capacity)
    {
        _resize_memory(size);
    }

    // add or remove
    if (size > _size)
    {
        add_unsafe(size - _size);
    }
    else if (size < _size)
    {
        remove_at(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resize_default(SizeType size)
{
    // need grow
    if (size > _capacity)
    {
        _resize_memory(size);
    }

    // add or remove
    if (size > _size)
    {
        add_default(size - _size);
    }
    else if (size < _size)
    {
        remove_at(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resize_zeroed(SizeType size)
{
    // need grow
    if (size > _capacity)
    {
        _resize_memory(size);
    }

    // add or remove
    if (size > _size)
    {
        add_zeroed(size - _size);
    }
    else if (size < _size)
    {
        remove_at(size, _size - size);
    }

    // set size
    _size = size;
}

// add
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add(const T& v, SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index; i < _size; ++i)
    {
        new (_data + i) T(v);
    }
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add(T&& v)
{
    DataRef ref = add_unsafe();
    new (ref.data) T(std::move(v));
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add_unique(const T& v)
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
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add_unsafe(SizeType n)
{
    auto old_size = _size;
    _grow(n);
    return DataRef(_data + old_size, old_size);
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add_default(SizeType n)
{
    DataRef ref = add_unsafe(n);
    for (SizeType i = ref.index; i < _size; ++i)
    {
        new (_data + i) T();
    }
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add_zeroed(SizeType n)
{
    DataRef ref = add_unsafe(n);
    std::memset(ref.data, 0, n * sizeof(T));
    return ref;
}

// add at
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::add_at(SizeType idx, const T& v, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (_data + idx + i) T(v);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::add_at(SizeType idx, T&& v)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx);
    new (_data + idx) T(std::move(v));
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::add_at_unsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    auto move_n = _size - idx;
    _grow(n);
    memory::move(_data + idx + n, _data + idx, move_n);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::add_at_default(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (_data + idx + i) T();
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::add_at_zeroed(SizeType idx, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    add_at_unsafe(idx, n);
    std::memset(_data + idx, 0, n * sizeof(T));
}

// emplace
template <typename T, typename Alloc>
template <typename... Args>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::emplace(Args&&... args)
{
    DataRef ref = add_unsafe();
    new (ref.data) T(std::forward<Args>(args)...);
    return ref;
}
template <typename T, typename Alloc>
template <typename... Args>
SKR_INLINE void Array<T, Alloc>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (_data + index) T(std::forward<Args>(args)...);
}

// append
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::append(const Array& arr)
{
    if (arr._size)
    {
        DataRef ref = add_unsafe(arr.size());
        memory::copy(ref.data, arr._data, arr._size);
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::append(std::initializer_list<T> init_list)
{
    if (init_list.size())
    {
        DataRef ref = add_unsafe(init_list.size());
        memory::copy(ref.data, init_list.begin(), init_list.size());
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::append(T* p, SizeType n)
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
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::append_at(SizeType idx, const Array& arr)
{
    SKR_ASSERT(is_valid_index(idx));
    if (arr._size)
    {
        add_at_unsafe(idx, arr._size);
        memory::copy(_data + idx, arr._data, arr._size);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::append_at(SizeType idx, std::initializer_list<T> init_list)
{
    SKR_ASSERT(is_valid_index(idx));
    if (init_list.size())
    {
        add_at_unsafe(idx, init_list.size());
        memory::copy(_data + idx, init_list.begin(), init_list.size());
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::append_at(SizeType idx, T* p, SizeType n)
{
    SKR_ASSERT(is_valid_index(idx));
    if (n)
    {
        add_at_unsafe(idx, n);
        memory::copy(_data + idx, p, n);
    }
}

// remove
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= _size);

    if (n)
    {
        // calc move size
        auto move_n = _size - index - n;

        // destruct remove items
        memory::destruct(_data + index, n);

        // move data
        if (move_n)
        {
            memory::move(_data + index, _data + _size - move_n, move_n);
        }

        // update size
        _size -= n;
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::remove_at_swap(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= _size);
    if (n)
    {
        // calc move size
        auto move_n = std::min(_size - index - n, n);

        // destruct remove items
        memory::destruct(_data + index, n);

        // move data
        if (move_n)
        {
            memory::move(_data + index, _data + _size - move_n, move_n);
        }

        // update size
        _size -= n;
    }
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove(const TK& v)
{
    if (DataRef ref = find(v))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_swap(const TK& v)
{
    if (DataRef ref = find(v))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_last(const TK& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_last_swap(const TK& v)
{
    if (DataRef ref = find_last(v))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::remove_all(const TK& v)
{
    return remove_all_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::remove_all_swap(const TK& v)
{
    return remove_all_if_swap([&v](const T& a) { return a == v; });
}

// remove by
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_if(TP&& p)
{
    if (DataRef ref = find_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_if_swap(TP&& p)
{
    if (DataRef ref = find_if(std::forward<TP>(p)))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_last_if(TP&& p)
{
    if (DataRef ref = find_last_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::remove_last_if_swap(TP&& p)
{
    if (DataRef ref = find_last_if(std::forward<TP>(p)))
    {
        remove_at_swap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::remove_all_if(TP&& p)
{
    T*       pos = algo::remove_all(begin(), end(), std::forward<TP>(p));
    SizeType n   = end() - pos;
    _size -= n;
    return n;
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::remove_all_if_swap(TP&& p)
{
    T*       pos = algo::remove_all_swap(begin(), end(), p);
    SizeType n   = end() - pos;
    _size -= n;
    return n;
}

// modify
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::operator[](SizeType index)
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::operator[](SizeType index) const
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::last(SizeType index)
{
    index = _size - index;
    SKR_ASSERT(is_valid_index(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::last(SizeType index) const
{
    index = _size - index;
    SKR_ASSERT(is_valid_index(index));
    return *(_data + index);
}

// find
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::find(const TK& v)
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::find_last(const TK& v)
{
    return find_last_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::find(const TK& v) const
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::find_last(const TK& v) const
{
    return find_last_if([&v](const T& a) { return a == v; });
}

// find by
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::find_if(TP&& p)
{
    T* ret = algo::find(begin(), end(), std::forward<TP>(p));
    return ret ? DataRef(ret, ret - data()) : DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::find_last_if(TP&& p)
{
    T* ret = algo::find_last(begin(), end(), std::forward<TP>(p));
    return ret ? DataRef(ret, ret - data()) : DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::find_if(TP&& p) const
{
    const T* ret = algo::find(begin(), end(), std::forward<TP>(p));
    return ret ? CDataRef(ret, ret - data()) : CDataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::find_last_if(TP&& p) const
{
    const T* ret = algo::find_last(begin(), end(), std::forward<TP>(p));
    return ret ? CDataRef(ret, ret - data()) : CDataRef();
}

// contain
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE bool Array<T, Alloc>::contain(const TK& v) const { return (bool)find(v); }
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE bool Array<T, Alloc>::contain_if(TP&& p) const
{
    return (bool)find_if(std::forward<TP>(p));
}

// sort
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::sort(TP&& p)
{
    algo::intro_sort(begin(), end(), std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::sort_stable(TP&& p)
{
    algo::merge_sort(begin(), end(), std::forward<TP>(p));
}

// support heap
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::heap_top() { return *_data; }
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heapify(TP&& p)
{
    algo::heapify(_data, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE bool Array<T, Alloc>::is_heap(TP&& p)
{
    return algo::is_heap(_data, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::heap_push(T&& v, TP&& p)
{
    DataRef ref = emplace(std::move(v));
    return algo::heap_sift_up(_data, (SizeType)0, ref.index, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::heap_push(const T& v, TP&& p)
{
    DataRef ref = add(v);
    return algo::heap_sift_up(_data, SizeType(0), ref.index, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heap_pop(TP&& p)
{
    remove_at_swap(0);
    algo::heap_sift_down(_data, (SizeType)0, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE T Array<T, Alloc>::heap_pop_get(TP&& p)
{
    T result = std::move(*_data);
    heap_pop(std::forward<TP>(p));
    return result;
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heap_remove_at(SizeType index, TP&& p)
{
    remove_at_swap(index);

    algo::heap_sift_down(_data, index, _size, std::forward<TP>(p));
    algo::heap_sift_up(_data, (SizeType)0, std::min(index, _size - 1), std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heap_sort(TP&& p)
{
    algo::heap_sort(_data, _size, std::forward<TP>(p));
}

// support stack
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::pop(SizeType n)
{
    SKR_ASSERT(n > 0);
    SKR_ASSERT(n <= _size);
    memory::destruct(_data + _size - n, n);
    _size -= n;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::pop_unsafe(SizeType n)
{
    SKR_ASSERT(n > 0);
    SKR_ASSERT(n <= _size);
    _size -= n;
}
template <typename T, typename Alloc>
SKR_INLINE T Array<T, Alloc>::pop_get()
{
    T result = std::move(*(_data + _size - 1));
    pop();
    return result;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::push(const T& v) { add(v); }
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::push(T&& v) { add(std::move(v)); }
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::top() { return *(_data + _size - 1); }
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::top() const { return *(_data + _size - 1); }
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::bottom() { return *_data; }
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::bottom() const { return *_data; }

// support foreach
template <typename T, typename Alloc>
SKR_INLINE T* Array<T, Alloc>::begin() { return _data; }
template <typename T, typename Alloc>
SKR_INLINE T* Array<T, Alloc>::end() { return _data + _size; }
template <typename T, typename Alloc>
SKR_INLINE const T* Array<T, Alloc>::begin() const { return _data; }
template <typename T, typename Alloc>
SKR_INLINE const T* Array<T, Alloc>::end() const { return _data + _size; }
} // namespace skr
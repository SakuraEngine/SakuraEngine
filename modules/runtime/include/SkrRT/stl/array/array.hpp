#pragma once
#include "SkrRT/base/config.hpp"
#include "SkrRT/algo/intro_sort.hpp"
#include "SkrRT/algo/merge_sort.hpp"
#include "SkrRT/algo/remove.hpp"
#include "SkrRT/algo/find.hpp"
#include "SkrRT/stl/allocator/allocator.hpp"
#include "SkrRT/stl/array/array_def.hpp"

// Array def
namespace skr
{
template <typename T, typename Alloc>
class Array
{
public:
    using SizeType = typename Alloc::SizeType;
    using DataRef  = ArrayDataRef<T, SizeType>;
    using CDataRef = ArrayDataRef<const T, SizeType>;

    // ctor & dtor
    Array(Alloc alloc = Alloc());
    Array(SizeType size, Alloc alloc = Alloc());
    Array(SizeType size, const T& v, Alloc alloc = Alloc());
    Array(const T* p, SizeType n, Alloc alloc = Alloc());
    Array(std::initializer_list<T> init_list, Alloc alloc = Alloc());
    ~Array();

    // copy & move
    Array(const Array& other, Alloc alloc = Alloc());
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
    bool isValidIndex(SizeType idx) const;
    bool isValidPointer(const T* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    void resize(SizeType size, const T& new_value);
    void resizeUnsafe(SizeType size);
    void resizeDefault(SizeType size);
    void resizeZeroed(SizeType size);

    // add
    DataRef add(const T& v, SizeType n = 1);
    DataRef add(T&& v);
    DataRef addUnique(const T& v);
    DataRef addUnsafe(SizeType n = 1);
    DataRef addDefault(SizeType n = 1);
    DataRef addZeroed(SizeType n = 1);

    // add at
    void addAt(SizeType idx, const T& v, SizeType n = 1);
    void addAt(SizeType idx, T&& v);
    void addAtUnsafe(SizeType idx, SizeType n = 1);
    void addAtDefault(SizeType idx, SizeType n = 1);
    void addAtZeroed(SizeType idx, SizeType n = 1);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    void emplaceAt(SizeType index, Args&&... args);

    // append
    DataRef append(const Array& arr);
    DataRef append(std::initializer_list<T> init_list);
    DataRef append(T* p, SizeType n);

    // append at
    void appendAt(SizeType idx, const Array& arr);
    void appendAt(SizeType idx, std::initializer_list<T> init_list);
    void appendAt(SizeType idx, T* p, SizeType n);

    // remove
    void removeAt(SizeType index, SizeType n = 1);
    void removeAtSwap(SizeType index, SizeType n = 1);
    template <typename TK>
    DataRef remove(const TK& v);
    template <typename TK>
    DataRef removeSwap(const TK& v);
    template <typename TK>
    DataRef removeLast(const TK& v);
    template <typename TK>
    DataRef removeLastSwap(const TK& v);
    template <typename TK>
    SizeType removeAll(const TK& v);
    template <typename TK>
    SizeType removeAllSwap(const TK& v);

    // remove if
    template <typename TP>
    DataRef removeIf(TP&& p);
    template <typename TP>
    DataRef removeIfSwap(TP&& p);
    template <typename TP>
    DataRef removeLastIf(TP&& p);
    template <typename TP>
    DataRef removeLastIfSwap(TP&& p);
    template <typename TP>
    SizeType removeAllIf(TP&& p);
    template <typename TP>
    SizeType removeAllIfSwap(TP&& p);

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;
    T&       last(SizeType index);
    const T& last(SizeType index) const;

    // find
    template <typename TK>
    DataRef find(const TK& v);
    template <typename TK>
    DataRef findLast(const TK& v);
    template <typename TK>
    CDataRef find(const TK& v) const;
    template <typename TK>
    CDataRef findLast(const TK& v) const;

    // find if
    template <typename TP>
    DataRef findIf(TP&& p);
    template <typename TP>
    DataRef findLastIf(TP&& p);
    template <typename TP>
    CDataRef findIf(TP&& p) const;
    template <typename TP>
    CDataRef findLastIf(TP&& p) const;

    // contain
    template <typename TK>
    bool contain(const TK& v) const;
    template <typename TP>
    bool containIf(TP&& p) const;

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = TP());
    template <typename TP = Less<T>>
    void sortStable(TP&& p = TP());

    // support heap
    T& heapTop();
    template <typename TP = Less<T>>
    void heapify(TP&& p = TP());
    template <typename TP = Less<T>>
    bool isHeap(TP&& p = TP());
    template <typename TP = Less<T>>
    SizeType heapPush(T&& v, TP&& p = TP());
    template <typename TP = Less<T>>
    SizeType heapPush(const T& v, TP&& p = TP());
    template <typename TP = Less<T>>
    void heapPop(TP&& p = TP());
    template <typename TP = Less<T>>
    T heapPopGet(TP&& p = TP());
    template <typename TP = Less<T>>
    void heapRemoveAt(SizeType index, TP&& p = TP());
    template <typename TP = Less<T>>
    void heapSort(TP&& p = TP());

    // support stack
    void     pop(SizeType n = 1);
    void     popUnsafe(SizeType n = 1);
    T        popGet();
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
    void _resizeMemory(SizeType new_capacity);
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
SKR_INLINE void Array<T, Alloc>::_resizeMemory(SizeType new_capacity)
{
    if (new_capacity)
    {
        // realloc
        _data     = _alloc.resizeContainer(_data, _size, _capacity, new_capacity);
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
    resizeDefault(size);
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
    resizeUnsafe(n);
    memory::copy(_data, p, n);
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(std::initializer_list<T> init_list, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resizeUnsafe(init_list.size());
    memory::copy(_data, init_list.begin(), init_list.size());
}
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::~Array() { release(); }

// copy & move
template <typename T, typename Alloc>
SKR_INLINE Array<T, Alloc>::Array(const Array& other, Alloc alloc)
    : _data(nullptr)
    , _size(0)
    , _capacity(0)
    , _alloc(std::move(alloc))
{
    resizeUnsafe(other.size());
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
        resizeUnsafe(rhs._size);

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
    resizeUnsafe(n);

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
SKR_INLINE bool Array<T, Alloc>::operator!=(const Array& rhs) const { return !(*this == rhs); }

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
SKR_INLINE bool Array<T, Alloc>::isValidIndex(SizeType idx) const { return idx >= 0 && idx < _size; }
template <typename T, typename Alloc>
SKR_INLINE bool Array<T, Alloc>::isValidPointer(const T* p) const { return p >= begin() && p < end(); }

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
    _resizeMemory(capacity);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::reserve(SizeType capacity)
{
    if (capacity > _capacity)
    {
        _resizeMemory(capacity);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::shrink()
{
    auto new_capacity = _alloc.getShrink(_size, _capacity);
    _resizeMemory(new_capacity);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resize(SizeType size, const T& new_value)
{
    // explicit size
    if (size > _capacity)
    {
        _resizeMemory(size);
    }

    // add or remove
    if (size > _size)
    {
        add(new_value, size - _size);
    }
    else if (size < _size)
    {
        removeAt(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resizeUnsafe(SizeType size)
{
    // explicit size
    if (size > _capacity)
    {
        _resizeMemory(size);
    }

    // add or remove
    if (size > _size)
    {
        addUnsafe(size - _size);
    }
    else if (size < _size)
    {
        removeAt(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resizeDefault(SizeType size)
{
    // explicit size
    if (size > _capacity)
    {
        _resizeMemory(size);
    }

    // add or remove
    if (size > _size)
    {
        addDefault(size - _size);
    }
    else if (size < _size)
    {
        removeAt(size, _size - size);
    }

    // set size
    _size = size;
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::resizeZeroed(SizeType size)
{
    // explicit size
    if (size > _capacity)
    {
        _resizeMemory(size);
    }

    // add or remove
    if (size > _size)
    {
        addZeroed(size - _size);
    }
    else if (size < _size)
    {
        removeAt(size, _size - size);
    }

    // set size
    _size = size;
}

// add
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add(const T& v, SizeType n)
{
    DataRef ref = addUnsafe(n);
    for (SizeType i = ref.index; i < _size; ++i)
    {
        new (_data + i) T(v);
    }
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::add(T&& v)
{
    DataRef ref = addUnsafe();
    new (ref.data) T(std::move(v));
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::addUnique(const T& v)
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
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::addUnsafe(SizeType n)
{
    auto old_size = _size;
    _grow(n);
    return DataRef(_data + old_size, old_size);
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::addDefault(SizeType n)
{
    DataRef ref = addUnsafe(n);
    for (SizeType i = ref.index; i < _size; ++i)
    {
        new (_data + i) T();
    }
    return ref;
}
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::addZeroed(SizeType n)
{
    DataRef ref = addUnsafe(n);
    std::memset(ref.data, 0, n * sizeof(T));
    return ref;
}

// add at
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::addAt(SizeType idx, const T& v, SizeType n)
{
    SKR_ASSERT(isValidIndex(idx));
    addAtUnsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (_data + idx + i) T(v);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::addAt(SizeType idx, T&& v)
{
    SKR_ASSERT(isValidIndex(idx));
    addAtUnsafe(idx);
    new (_data + idx) T(std::move(v));
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::addAtUnsafe(SizeType idx, SizeType n)
{
    SKR_ASSERT(isValidIndex(idx));
    auto move_n = _size - idx;
    _grow(n);
    memory::move(_data + idx + n, _data + idx, move_n);
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::addAtDefault(SizeType idx, SizeType n)
{
    SKR_ASSERT(isValidIndex(idx));
    addAtUnsafe(idx, n);
    for (SizeType i = 0; i < n; ++i)
    {
        new (_data + idx + i) T();
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::addAtZeroed(SizeType idx, SizeType n)
{
    SKR_ASSERT(isValidIndex(idx));
    addAtUnsafe(idx, n);
    std::memset(_data + idx, 0, n * sizeof(T));
}

// emplace
template <typename T, typename Alloc>
template <typename... Args>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::emplace(Args&&... args)
{
    DataRef ref = addUnsafe();
    new (ref.data) T(std::forward<Args>(args)...);
    return ref;
}
template <typename T, typename Alloc>
template <typename... Args>
SKR_INLINE void Array<T, Alloc>::emplaceAt(SizeType index, Args&&... args)
{
    addAtUnsafe(index);
    new (_data + index) T(std::forward<Args>(args)...);
}

// append
template <typename T, typename Alloc>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::append(const Array& arr)
{
    if (arr._size)
    {
        DataRef ref = addUnsafe(arr.size());
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
        DataRef ref = addUnsafe(init_list.size());
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
        DataRef ref = addUnsafe(n);
        memory::copy(ref.data, p, n);
        return ref;
    }
    return data() ? DataRef(data() + size(), size()) : DataRef();
}

// append at
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::appendAt(SizeType idx, const Array& arr)
{
    SKR_ASSERT(isValidIndex(idx));
    if (arr._size)
    {
        addAtUnsafe(idx, arr._size);
        memory::copy(_data + idx, arr._data, arr._size);
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::appendAt(SizeType idx, std::initializer_list<T> init_list)
{
    SKR_ASSERT(isValidIndex(idx));
    if (init_list.size())
    {
        addAtUnsafe(idx, init_list.size());
        memory::copy(_data + idx, init_list.begin(), init_list.size());
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::appendAt(SizeType idx, T* p, SizeType n)
{
    SKR_ASSERT(isValidIndex(idx));
    if (n)
    {
        addAtUnsafe(idx, n);
        memory::copy(_data + idx, p, n);
    }
}

// remove
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::removeAt(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= _size);

    if (n)
    {
        // calc move size
        auto move_n = _size - index - n;

        // move data
        if (move_n)
        {
            memory::move(_data + index, _data + _size - move_n, move_n);
        }

        // destruct data
        memory::destruct(_data + index + move_n, n);

        // update size
        _size -= n;
    }
}
template <typename T, typename Alloc>
SKR_INLINE void Array<T, Alloc>::removeAtSwap(SizeType index, SizeType n)
{
    SKR_ASSERT(index >= 0 && index + n <= _size);
    if (n)
    {
        // calc move size
        auto move_n = std::min(_size - index - n, n);

        // move data
        if (move_n)
        {
            memory::move(_data + index, _data + _size - move_n, move_n);
        }

        // destruct data
        memory::destruct(_data + index + move_n, n);

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
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeSwap(const TK& v)
{
    if (DataRef ref = find(v))
    {
        removeAtSwap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeLast(const TK& v)
{
    if (DataRef ref = findLast(v))
    {
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeLastSwap(const TK& v)
{
    if (DataRef ref = findLast(v))
    {
        removeAtSwap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::removeAll(const TK& v)
{
    return removeAllIf([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::removeAllSwap(const TK& v)
{
    return removeAllIfSwap([&v](const T& a) { return a == v; });
}

// remove by
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeIf(TP&& p)
{
    if (DataRef ref = findIf(std::forward<TP>(p)))
    {
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeIfSwap(TP&& p)
{
    if (DataRef ref = findIf(std::forward<TP>(p)))
    {
        removeAtSwap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeLastIf(TP&& p)
{
    if (DataRef ref = findLastIf(std::forward<TP>(p)))
    {
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::removeLastIfSwap(TP&& p)
{
    if (DataRef ref = findLastIf(std::forward<TP>(p)))
    {
        removeAtSwap(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::removeAllIf(TP&& p)
{
    T*       pos = algo::remove_all(begin(), end(), std::forward<TP>(p));
    SizeType n   = end() - pos;
    if (n)
    {
        memory::destruct(pos, n);
    }
    _size -= n;
    return n;
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::removeAllIfSwap(TP&& p)
{
    T*       pos = algo::remove_all_swap(begin(), end(), p);
    SizeType n   = end() - pos;
    if (n)
    {
        memory::destruct(pos, n);
    }
    _size -= n;
    return n;
}

// modify
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::operator[](SizeType index)
{
    SKR_ASSERT(isValidIndex(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::operator[](SizeType index) const
{
    SKR_ASSERT(isValidIndex(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::last(SizeType index)
{
    index = _size - index;
    SKR_ASSERT(isValidIndex(index));
    return *(_data + index);
}
template <typename T, typename Alloc>
SKR_INLINE const T& Array<T, Alloc>::last(SizeType index) const
{
    index = _size - index;
    SKR_ASSERT(isValidIndex(index));
    return *(_data + index);
}

// find
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::find(const TK& v)
{
    return findIf([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::findLast(const TK& v)
{
    return findLastIf([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::find(const TK& v) const
{
    return findIf([&v](const T& a) { return a == v; });
}
template <typename T, typename Alloc>
template <typename TK>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::findLast(const TK& v) const
{
    return findLastIf([&v](const T& a) { return a == v; });
}

// find by
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::findIf(TP&& p)
{
    T* ret = algo::find(begin(), end(), std::forward<TP>(p));
    return ret ? DataRef(ret, ret - data()) : DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::DataRef Array<T, Alloc>::findLastIf(TP&& p)
{
    T* ret = algo::find_last(begin(), end(), std::forward<TP>(p));
    return ret ? DataRef(ret, ret - data()) : DataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::findIf(TP&& p) const
{
    const T* ret = algo::find(begin(), end(), std::forward<TP>(p));
    return ret ? CDataRef(ret, ret - data()) : CDataRef();
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::CDataRef Array<T, Alloc>::findLastIf(TP&& p) const
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
SKR_INLINE bool Array<T, Alloc>::containIf(TP&& p) const
{
    return (bool)findIf(std::forward<TP>(p));
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
SKR_INLINE void Array<T, Alloc>::sortStable(TP&& p)
{
    algo::merge_sort(begin(), end(), std::forward<TP>(p));
}

// support heap
template <typename T, typename Alloc>
SKR_INLINE T& Array<T, Alloc>::heapTop() { return *_data; }
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heapify(TP&& p)
{
    algo::heapify(_data, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE bool Array<T, Alloc>::isHeap(TP&& p)
{
    return algo::is_heap(_data, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::heapPush(T&& v, TP&& p)
{
    DataRef ref = emplace(std::move(v));
    return algo::heap_sift_up(_data, (SizeType)0, ref.index, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE typename Array<T, Alloc>::SizeType Array<T, Alloc>::heapPush(const T& v, TP&& p)
{
    DataRef ref = add(v);
    return algo::heap_sift_up(_data, SizeType(0), ref.index, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heapPop(TP&& p)
{
    removeAtSwap(0);
    algo::heap_sift_down(_data, (SizeType)0, _size, std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE T Array<T, Alloc>::heapPopGet(TP&& p)
{
    T result = std::move(*_data);
    heapPop(std::forward<TP>(p));
    return result;
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heapRemoveAt(SizeType index, TP&& p)
{
    removeAtSwap(index);

    algo::heap_sift_down(_data, index, _size, std::forward<TP>(p));
    algo::heap_sift_up(_data, (SizeType)0, std::min(index, _size - 1), std::forward<TP>(p));
}
template <typename T, typename Alloc>
template <typename TP>
SKR_INLINE void Array<T, Alloc>::heapSort(TP&& p)
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
SKR_INLINE void Array<T, Alloc>::popUnsafe(SizeType n)
{
    SKR_ASSERT(n > 0);
    SKR_ASSERT(n <= _size);
    _size -= n;
}
template <typename T, typename Alloc>
SKR_INLINE T Array<T, Alloc>::popGet()
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
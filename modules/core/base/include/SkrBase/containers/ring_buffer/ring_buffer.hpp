#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/ring_buffer/ring_buffer_def.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_iterator.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_heler.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"

// RingBuffer def
namespace skr::container
{
template <typename Memory>
struct RingBuffer : protected Memory {
    // from memory
    using DataType           = typename Memory::DataType;
    using SizeType           = typename Memory::SizeType;
    using AllocatorCtorParam = typename Memory::AllocatorCtorParam;

    // data ref and iterator
    using DataRef  = RingBufferDataRef<DataType, SizeType, false>;
    using CDataRef = RingBufferDataRef<DataType, SizeType, true>;
    using StlIt    = RingBufferIt<DataType, SizeType, false>;
    using CStlIt   = RingBufferIt<const DataType, SizeType, true>;

    // ctor & dtor
    RingBuffer(AllocatorCtorParam param = {});
    RingBuffer(SizeType expect_capacity, AllocatorCtorParam param = {});
    RingBuffer(const DataType* p, SizeType n, AllocatorCtorParam param = {});
    RingBuffer(std::initializer_list<DataType> init_list, AllocatorCtorParam param = {});
    ~RingBuffer();

    // copy & move
    RingBuffer(const RingBuffer& other);
    RingBuffer(RingBuffer&& other) noexcept;

    // assign & move assign
    RingBuffer& operator=(const RingBuffer& rhs);
    RingBuffer& operator=(RingBuffer&& rhs) noexcept;

    // special assign
    void assign(const DataType* p, SizeType n);
    void assign(std::initializer_list<DataType> init_list);

    // getter
    SizeType      size() const;
    SizeType      capacity() const;
    SizeType      slack() const;
    bool          is_empty() const;
    bool          full() const;
    Memory&       memory();
    const Memory& memory() const;

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

    // push
    DataRef push_back(const DataType& v, SizeType n = 1);
    DataRef push_back(DataType&& v);
    DataRef push_back_unsafe(SizeType n = 1);
    DataRef push_back_default(SizeType n = 1);
    DataRef push_back_zeroed(SizeType n = 1);
    DataRef push_front(const DataType& v, SizeType n = 1);
    DataRef push_front(DataType&& v);
    DataRef push_front_unsafe(SizeType n = 1);
    DataRef push_front_default(SizeType n = 1);
    DataRef push_front_zeroed(SizeType n = 1);

    // emplace
    template <typename... Args>
    requires(std::is_constructible_v<typename Memory::DataType, Args...>)
    DataRef emplace_back(Args&&... args);
    template <typename... Args>
    requires(std::is_constructible_v<typename Memory::DataType, Args...>)
    DataRef emplace_front(Args&&... args);

    // append
    DataRef append_back(const DataType* p, SizeType n);
    DataRef append_back(std::initializer_list<DataType> init_list);
    DataRef append_front(const DataType* p, SizeType n);
    DataRef append_front(std::initializer_list<DataType> init_list);

    // pop
    void     pop_back(SizeType n = 1);
    void     pop_back_unsafe(SizeType n = 1);
    DataType pop_back_get();
    void     pop_front(SizeType n = 1);
    void     pop_front_unsafe(SizeType n = 1);
    DataType pop_front_get();

    // modify
    DataType&       operator[](SizeType index);
    const DataType& operator[](SizeType index) const;
    DataType&       at(SizeType index);
    const DataType& at(SizeType index) const;
    DataType&       at_last(SizeType index);
    const DataType& at_last(SizeType index) const;

    // front/back
    DataType&       front();
    const DataType& front() const;
    DataType&       back();
    const DataType& back() const;

    // support foreach
    StlIt  begin();
    CStlIt begin() const;
    StlIt  end();
    CStlIt end() const;

    // syntax
    const RingBuffer& readonly() const;

private:
    // helper
    DataType*       _data();
    const DataType* _data() const;
    void            _realloc(SizeType expect_capacity);
    void            _free();
    SizeType        _grow_back(SizeType n);
    SizeType        _grow_front(SizeType n);
    void            _set_front(SizeType value);
    void            _set_back(SizeType value);
    SizeType        _front() const;
    SizeType        _back() const;
    void            _rearrange_for_push_front(SizeType n);
    void            _rearrange_for_push_back(SizeType n);
    void            _construct_default(SizeType front, SizeType back);
    void            _construct_value(SizeType front, SizeType back, const DataType& v);
    void            _construct_stl_ub(SizeType front, SizeType back);
    void            _construct_zeroed(SizeType front, SizeType back);
};
} // namespace skr::container

// RingBuffer impl
namespace skr::container
{
// helper
template <typename Memory>
inline typename RingBuffer<Memory>::DataType* RingBuffer<Memory>::_data()
{
    return Memory::data();
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType* RingBuffer<Memory>::_data() const
{
    return Memory::data();
}
template <typename Memory>
inline void RingBuffer<Memory>::_realloc(SizeType expect_capacity)
{
    Memory::realloc(expect_capacity);
}
template <typename Memory>
inline void RingBuffer<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::_grow_back(SizeType n)
{
    Memory::grow_memory(n);
    _rearrange_for_push_back(n);

    SizeType old_back = _back();
    _set_back(_back() + n);
    return old_back;
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::_grow_front(SizeType n)
{
    Memory::grow_memory(n);
    _rearrange_for_push_front(n);

    SizeType old_front = _front();
    _set_front(_front() - n);
    return old_front;
}
template <typename Memory>
inline void RingBuffer<Memory>::_set_front(SizeType value)
{
    Memory::set_front(value);
}
template <typename Memory>
inline void RingBuffer<Memory>::_set_back(SizeType value)
{
    Memory::set_back(value);
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::_front() const
{
    return Memory::front();
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::_back() const
{
    return Memory::back();
}
template <typename Memory>
inline void RingBuffer<Memory>::_rearrange_for_push_front(SizeType n)
{
    SKR_ASSERT(size() + n <= capacity() && "size() + n must less than capacity()");

    if (_front() < n)
    {
        _set_front(_front() + capacity());
        _set_back(_back() + capacity());
    }
}
template <typename Memory>
inline void RingBuffer<Memory>::_rearrange_for_push_back(SizeType n)
{
    SKR_ASSERT(size() + n <= capacity() && "size() + n must less than capacity()");

    if (std::numeric_limits<SizeType>::max() - _back() <= n)
    {
        SizeType record_size = size();
        _set_front(_front() % capacity());
        _set_back(_front() + record_size);
    }
}
template <typename Memory>
inline void RingBuffer<Memory>::_construct_default(SizeType front, SizeType back)
{
    process_ring_buffer_data(
        capacity(),
        front,
        back,
        [this](SizeType dst_idx, SizeType src_idx, SizeType size) {
            ::skr::memory::construct(_data() + src_idx, size);
        }
    );
}
template <typename Memory>
inline void RingBuffer<Memory>::_construct_value(SizeType front, SizeType back, const DataType& v)
{
    process_ring_buffer_data(
        capacity(),
        front,
        back,
        [this, &v](SizeType dst_idx, SizeType src_idx, SizeType size) {
            for (SizeType i = 0; i < size; ++i)
            {
                new (_data() + src_idx + i) DataType(v);
            }
        }
    );
}
template <typename Memory>
inline void RingBuffer<Memory>::_construct_zeroed(SizeType front, SizeType back)
{
    process_ring_buffer_data(
        capacity(),
        front,
        back,
        [this](SizeType dst_idx, SizeType src_idx, SizeType size) {
            std::memset(_data() + src_idx, 0, (size * sizeof(DataType)));
        }
    );
}

// ctor & dtor
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(AllocatorCtorParam param)
    : Memory(std::move(param))
{
}
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(SizeType expect_capacity, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    reserve(expect_capacity);
}
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(const DataType* p, SizeType n, AllocatorCtorParam param)
    : Memory(param)
{
    _realloc(n);
    _set_back(n);
    ::skr::memory::copy(_data(), p, n);
}
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(std::initializer_list<DataType> init_list, AllocatorCtorParam param)
    : Memory(param)
{
    _realloc(init_list.size());
    _set_back(init_list.size());
    ::skr::memory::copy(_data(), init_list.begin(), init_list.size());
}
template <typename Memory>
inline RingBuffer<Memory>::~RingBuffer()
{
    // handled in memory
}

// copy & move
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(const RingBuffer& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
inline RingBuffer<Memory>::RingBuffer(RingBuffer&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
inline RingBuffer<Memory>& RingBuffer<Memory>::operator=(const RingBuffer& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
inline RingBuffer<Memory>& RingBuffer<Memory>::operator=(RingBuffer&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// special assign
template <typename Memory>
inline void RingBuffer<Memory>::assign(const DataType* p, SizeType n)
{
    // clear and resize
    clear();
    resize_unsafe(n);

    // copy items
    ::skr::memory::copy(_data(), p, n);
}
template <typename Memory>
inline void RingBuffer<Memory>::assign(std::initializer_list<DataType> init_list)
{
    assign(init_list.begin(), init_list.size());
}

// getter
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
inline typename RingBuffer<Memory>::SizeType RingBuffer<Memory>::slack() const
{
    return capacity() - size();
}
template <typename Memory>
inline bool RingBuffer<Memory>::is_empty() const
{
    return size() == 0;
}
template <typename Memory>
inline bool RingBuffer<Memory>::full() const
{
    return slack() == 0;
}
template <typename Memory>
inline Memory& RingBuffer<Memory>::memory()
{
    return *this;
}
template <typename Memory>
inline const Memory& RingBuffer<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
inline bool RingBuffer<Memory>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < size();
}

// memory op
template <typename Memory>
inline void RingBuffer<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
inline void RingBuffer<Memory>::release(SizeType reserve_capacity)
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
inline void RingBuffer<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        if (expect_capacity != capacity())
        {
            _realloc(expect_capacity);
        }
    }
    else
    {
        _free();
    }
}
template <typename Memory>
inline void RingBuffer<Memory>::shrink()
{
    Memory::shrink();
}
template <typename Memory>
inline void RingBuffer<Memory>::resize(SizeType expect_size, const DataType& new_value)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        _rearrange_for_push_back(expect_size - size());

        _construct_value(_back(), _front() + expect_size, new_value);
    }
    else if (expect_size < size())
    {
        destruct_ring_buffer(_data(), capacity(), _front() + expect_size, _back());
    }

    // set back
    _set_back(_front() + expect_size);
}
template <typename Memory>
inline void RingBuffer<Memory>::resize_unsafe(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // destruct item if need
    if (expect_size < size())
    {
        destruct_ring_buffer(_data(), capacity(), _front() + expect_size, _back());
    }

    // set back
    _set_back(_front() + expect_size);
}
template <typename Memory>
inline void RingBuffer<Memory>::resize_default(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        _rearrange_for_push_back(expect_size - size());

        _construct_default(_back(), _front() + expect_size);
    }
    else if (expect_size < size())
    {
        destruct_ring_buffer(_data(), capacity(), _front() + expect_size, _back());
    }

    // set back
    _set_back(_front() + expect_size);
}
template <typename Memory>
inline void RingBuffer<Memory>::resize_zeroed(SizeType expect_size)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > size())
    {
        _rearrange_for_push_back(expect_size - size());

        _construct_zeroed(_back(), _front() + expect_size);
    }
    else if (expect_size < size())
    {
        destruct_ring_buffer(_data(), capacity(), _front() + expect_size, _back());
    }

    // set back
    _set_back(_front() + expect_size);
}

// push
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_back(const DataType& v, SizeType n)
{
    [[maybe_unused]] SizeType old_back = _grow_back(n);
    _construct_value(old_back, _back(), v);
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_back(DataType&& v)
{
    [[maybe_unused]] SizeType old_back = _grow_back(1);
    DataType*                 ptr      = _data() + (old_back % capacity());
    new (ptr) DataType(std::move(v));
    return { ptr, old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_back_unsafe(SizeType n)
{
    [[maybe_unused]] SizeType old_back = _grow_back(n);
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_back_default(SizeType n)
{
    [[maybe_unused]] SizeType old_back = _grow_back(n);
    _construct_default(old_back, _back());
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_back_zeroed(SizeType n)
{
    [[maybe_unused]] SizeType old_back = _grow_back(n);
    _construct_zeroed(old_back, _back());
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_front(const DataType& v, SizeType n)
{
    [[maybe_unused]] SizeType old_front = _grow_front(n);
    _construct_value(_front(), old_front, v);
    return { _data() + (_front() % capacity()), 0 };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_front(DataType&& v)
{
    [[maybe_unused]] SizeType old_front = _grow_front(1);
    DataType*                 ptr       = _data() + (_front() % capacity());
    new (ptr) DataType(std::move(v));
    return { ptr, 0 };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_front_unsafe(SizeType n)
{
    [[maybe_unused]] SizeType old_front = _grow_front(n);
    return { _data() + (_front() % capacity()), 0 };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_front_default(SizeType n)
{
    [[maybe_unused]] SizeType old_front = _grow_front(n);
    _construct_default(_front(), old_front);
    return { _data() + (_front() % capacity()), 0 };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::push_front_zeroed(SizeType n)
{
    [[maybe_unused]] SizeType old_front = _grow_front(n);
    _construct_zeroed(_front(), old_front);
    return { _data() + (_front() % capacity()), 0 };
}

// emplace
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::DataType, Args...>)
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::emplace_back(Args&&... args)
{
    DataRef ref = push_back_unsafe(1);
    new (ref.ptr()) DataType(std::forward<Args>(args)...);
    return ref;
}
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::DataType, Args...>)
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::emplace_front(Args&&... args)
{
    DataRef ref = push_front_unsafe(1);
    new (ref.ptr()) DataType(std::forward<Args>(args)...);
    return ref;
}

// append
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::append_back(const DataType* p, SizeType n)
{
    SizeType old_back = _grow_back(n);
    process_ring_buffer_data(
        capacity(),
        old_back,
        _back(),
        [this, &p](SizeType dst_idx, SizeType src_idx, SizeType size) {
            ::skr::memory::copy(_data() + src_idx, p + dst_idx, size);
        }
    );
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::append_back(std::initializer_list<DataType> init_list)
{
    SizeType old_back = _grow_back(init_list.size());
    process_ring_buffer_data(
        capacity(),
        old_back,
        _back(),
        [this, &init_list](SizeType dst_idx, SizeType src_idx, SizeType size) {
            ::skr::memory::copy(_data() + src_idx, init_list.begin() + dst_idx, size);
        }
    );
    return { _data() + (old_back % capacity()), old_back - _front() };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::append_front(const DataType* p, SizeType n)
{
    SizeType old_front = _grow_front(n);
    process_ring_buffer_data(
        capacity(),
        _front(),
        old_front,
        [this, &p](SizeType dst_idx, SizeType src_idx, SizeType size) {
            ::skr::memory::copy(_data() + src_idx, p + dst_idx, size);
        }
    );
    return { _data() + (_front() % capacity()), 0 };
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataRef RingBuffer<Memory>::append_front(std::initializer_list<DataType> init_list)
{
    SizeType old_front = _grow_front(init_list.size());
    process_ring_buffer_data(
        capacity(),
        _front(),
        old_front,
        [this, &init_list](SizeType dst_idx, SizeType src_idx, SizeType size) {
            ::skr::memory::copy(_data() + src_idx, init_list.begin() + dst_idx, size);
        }
    );
    return { _data() + (_front() % capacity()), 0 };
}

// pop
template <typename Memory>
inline void RingBuffer<Memory>::pop_back(SizeType n)
{
    SKR_ASSERT(size() >= n && "pop_back() on empty buffer");

    SizeType old_back = _back();
    _set_back(old_back - n);
    ::skr::memory::destruct(_data() + (_back() % capacity()), n);
}
template <typename Memory>
inline void RingBuffer<Memory>::pop_back_unsafe(SizeType n)
{
    SKR_ASSERT(size() >= n && "pop_back_unsafe() on empty buffer");

    _set_back(_back() - n);
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataType RingBuffer<Memory>::pop_back_get()
{
    SKR_ASSERT(size() > 0 && "pop_back_get() on empty buffer");

    DataType result = std::move(*(_data() + ((_back() - 1) % capacity())));
    _set_back(_back() - 1);
    return std::move(result);
}
template <typename Memory>
inline void RingBuffer<Memory>::pop_front(SizeType n)
{
    SKR_ASSERT(size() >= n && "pop_front() on empty buffer");

    SizeType old_front = _front();
    _set_front(old_front + n);
    ::skr::memory::destruct(_data() + (old_front % capacity()), n);
}
template <typename Memory>
inline void RingBuffer<Memory>::pop_front_unsafe(SizeType n)
{
    SKR_ASSERT(size() >= n && "pop_front_unsafe() on empty buffer");

    _set_front(_front() + n);
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataType RingBuffer<Memory>::pop_front_get()
{
    SKR_ASSERT(size() > 0 && "pop_front_get() on empty buffer");

    DataType result = std::move(*(_data() + (_front() % capacity())));
    _set_front(_front() + 1);
    return std::move(result);
}

// modify
template <typename Memory>
inline typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::operator[](SizeType index)
{
    return at(index);
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::operator[](SizeType index) const
{
    return at(index);
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::at(SizeType index)
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data() + ((_front() + index) % capacity()));
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::at(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data() + ((_front() + index) % capacity()));
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::at_last(SizeType index)
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data() + ((_back() - index - 1) % capacity()));
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::at_last(SizeType index) const
{
    SKR_ASSERT(is_valid_index(index));
    return *(_data() + ((_back() - index - 1) % capacity()));
}

// front/back
template <typename Memory>
inline typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::front()
{
    SKR_ASSERT(size() > 0 && "visit an empty buffer");
    return *(_data() + (_front() % capacity()));
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::front() const
{
    SKR_ASSERT(size() > 0 && "visit an empty buffer");
    return *(_data() + (_front() % capacity()));
}
template <typename Memory>
inline typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::back()
{
    SKR_ASSERT(size() > 0 && "visit an empty buffer");
    return *(_data() + ((_back() - 1) % capacity()));
}
template <typename Memory>
inline const typename RingBuffer<Memory>::DataType& RingBuffer<Memory>::back() const
{
    SKR_ASSERT(size() > 0 && "visit an empty buffer");
    return *(_data() + ((_back() - 1) % capacity()));
}

// support foreach
template <typename Memory>
inline typename RingBuffer<Memory>::StlIt RingBuffer<Memory>::begin()
{
    return StlIt(_data(), capacity(), _front());
}
template <typename Memory>
inline typename RingBuffer<Memory>::CStlIt RingBuffer<Memory>::begin() const
{
    return CStlIt(_data(), capacity(), _front());
}
template <typename Memory>
inline typename RingBuffer<Memory>::StlIt RingBuffer<Memory>::end()
{
    return StlIt(_data(), capacity(), _back());
}
template <typename Memory>
inline typename RingBuffer<Memory>::CStlIt RingBuffer<Memory>::end() const
{
    return CStlIt(_data(), capacity(), _back());
}

// syntax
template <typename Memory>
SKR_INLINE const RingBuffer<Memory>& RingBuffer<Memory>::readonly() const
{
    return *this;
}

} // namespace skr::container

// container traits
namespace skr::container
{
template <typename Memory>
struct ContainerTraits<RingBuffer<Memory>> {
    constexpr static bool is_linear_memory = false; // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = true;  // reserve()

    using ElementType = typename Memory::DataType;
    using SizeType    = typename Memory::SizeType;

    inline static SizeType size(const RingBuffer<Memory>& container) { return container.size(); }

    inline static typename RingBuffer<Memory>::StlIt  begin(RingBuffer<Memory>& container) { return container.begin(); }
    inline static typename RingBuffer<Memory>::StlIt  end(RingBuffer<Memory>& container) { return container.end(); }
    inline static typename RingBuffer<Memory>::CStlIt begin(const RingBuffer<Memory>& container) { return container.begin(); }
    inline static typename RingBuffer<Memory>::CStlIt end(const RingBuffer<Memory>& container) { return container.end(); }

    inline static void reserve(RingBuffer<Memory>& container, SizeType expect_capacity) { container.reserve(expect_capacity); }
};
} // namespace skr::container
#pragma once
#include "./sp_util.hpp"
#include <SkrBase/misc/hash.hpp>

namespace skr
{
template <typename T>
struct UPtr;
template <typename T>
struct SP;
template <typename T>
struct SPWeak;

// 保守 unique pointer 实现，允许协变，但是不处理虚析构情况，禁止 custom deleter，只用于 RAII 的内存管理
template <typename T>
struct UPtr {
    // ctor & dtor
    UPtr();
    UPtr(std::nullptr_t);
    UPtr(T* ptr);
    template <SPConvertible<T> U>
    UPtr(U* ptr);
    ~UPtr();

    // copy & move
    UPtr(const UPtr& rhs) = delete;
    UPtr(UPtr&& rhs);
    template <SPConvertible<T> U>
    UPtr(UPtr<U>&& rhs);

    // assign & move assign
    UPtr& operator=(std::nullptr_t);
    UPtr& operator=(const UPtr& rhs) = delete;
    UPtr& operator=(UPtr&& rhs);
    template <SPConvertible<T> U>
    UPtr& operator=(UPtr<U>&& rhs);

    // factory
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    static UPtr New(Args&&... args);
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    static UPtr NewZeroed(Args&&... args);

    // getter
    T* get() const;

    // empty
    bool is_empty() const;
    operator bool() const;

    // ops
    void reset();
    void reset(T* ptr);
    template <SPConvertible<T> U>
    void reset(U* ptr);
    T*   release();
    void swap(UPtr& rhs);

    // pointer behaviour
    T* operator->() const;
    T& operator*() const;

    // skr hash
    static size_t _skr_hash(const UPtr& obj);
    static size_t _skr_hash(T* ptr);

private:
    T* _ptr = nullptr;
};

// shared pointer
template <typename T>
struct SP {
    friend struct SPWeak<T>;

    // ctor & dtor
    SP();
    SP(std::nullptr_t);
    SP(T* ptr);
    SP(T* ptr, SPRefCounter* counter);
    template <SPConvertible<T> U>
    SP(U* ptr);
    template <SPConvertible<T> U>
    SP(UPtr<U>&& rhs);
    ~SP();

    // copy & move
    SP(const SP& rhs);
    SP(SP&& rhs);
    template <SPConvertible<T> U>
    SP(const SP<U>& rhs);
    template <SPConvertible<T> U>
    SP(SP<U>&& rhs);

    // assign & move assign
    SP& operator=(std::nullptr_t);
    SP& operator=(T* ptr);
    SP& operator=(const SP& rhs);
    SP& operator=(SP&& rhs);
    template <SPConvertible<T> U>
    SP& operator=(U* ptr);
    template <SPConvertible<T> U>
    SP& operator=(const SP<U>& rhs);
    template <SPConvertible<T> U>
    SP& operator=(SP<U>&& rhs);
    template <SPConvertible<T> U>
    SP& operator=(UPtr<U>&& rhs);

    // factory
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    static SP New(Args&&... args);
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    static SP NewZeroed(Args&&... args);

    // getter
    T* get() const;

    // count getter
    SPCounterType ref_count() const;
    SPCounterType ref_count_weak() const;
    SPRefCounter* get_counter() const;

    // empty
    bool is_empty() const;
    operator bool() const;

    // ops
    void reset();
    void reset(T* ptr);
    template <SPConvertible<T> U>
    void reset(U* ptr);
    void swap(SP& rhs);

    // pointer behaviour
    T* operator->() const;
    T& operator*() const;

    // cast
    template <typename U>
    SP<U> cast_static() const;

    // skr hash
    static size_t _skr_hash(const SP& obj);
    static size_t _skr_hash(T* ptr);

private:
    // helper
    void _release();

private:
    T*            _ptr     = nullptr;
    SPRefCounter* _counter = nullptr;
};

// weak pointer
template <typename T>
struct SPWeak {
    // ctor & dtor
    SPWeak();
    SPWeak(std::nullptr_t);
    SPWeak(T* ptr, SPRefCounter* counter);
    template <SPConvertible<T> U>
    SPWeak(const SP<U>& ptr);
    ~SPWeak();

    // copy & move
    SPWeak(const SPWeak& rhs);
    SPWeak(SPWeak&& rhs);
    template <SPConvertible<T> U>
    SPWeak(const SPWeak<U>& rhs);
    template <SPConvertible<T> U>
    SPWeak(SPWeak<U>&& rhs);

    // assign & move assign
    SPWeak& operator=(std::nullptr_t);
    SPWeak& operator=(const SPWeak& rhs);
    SPWeak& operator=(SPWeak&& rhs);
    template <SPConvertible<T> U>
    SPWeak& operator=(const SPWeak<U>& rhs);
    template <SPConvertible<T> U>
    SPWeak& operator=(SPWeak<U>&& rhs);
    template <SPConvertible<T> U>
    SPWeak& operator=(const SP<U>& rhs);

    // unsafe getter
    T*            get_unsafe() const;
    SPRefCounter* get_counter() const;

    // count getter
    SPCounterType ref_count_weak() const;

    // lock
    SP<T> lock() const;

    // empty
    bool is_empty() const;
    bool is_expired() const;
    bool is_alive() const;
    operator bool() const;

    // ops
    void reset();
    template <SPConvertible<T> U>
    void reset(const SP<U>& ptr);
    void swap(SPWeak& rhs);

    // cast
    template <typename U>
    SPWeak<U> cast_static() const;

    // skr hash
    static size_t _skr_hash(const SPWeak& obj);

private:
    T*            _ptr     = nullptr;
    SPRefCounter* _counter = nullptr;
};
} // namespace skr

// impl UPtr
namespace skr
{
// ctor & dtor
template <typename T>
inline UPtr<T>::UPtr()
{
}
template <typename T>
inline UPtr<T>::UPtr(std::nullptr_t)
{
}
template <typename T>
inline UPtr<T>::UPtr(T* ptr)
    : _ptr(ptr)
{
}
template <typename T>
template <SPConvertible<T> U>
inline UPtr<T>::UPtr(U* ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (ptr)
    {
        _ptr = static_cast<T*>(ptr);
    }
}
template <typename T>
inline UPtr<T>::~UPtr()
{
    reset();
}

// copy & move
template <typename T>
inline UPtr<T>::UPtr(UPtr&& rhs)
    : _ptr(rhs._ptr)
{
    rhs._ptr = nullptr;
}
template <typename T>
template <SPConvertible<T> U>
inline UPtr<T>::UPtr(UPtr<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        reset(static_cast<T*>(rhs.release()));
    }
}

// assign & move assign
template <typename T>
inline UPtr<T>& UPtr<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}
template <typename T>
inline UPtr<T>& UPtr<T>::operator=(UPtr&& rhs)
{
    if (!rhs.is_empty())
    {
        reset(rhs._ptr);
        rhs._ptr = nullptr;
    }
    else
    {
        reset();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline UPtr<T>& UPtr<T>::operator=(UPtr<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        reset(static_cast<T*>(rhs.release()));
    }
    else
    {
        reset();
    }
    return *this;
}

// factory
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline UPtr<T> UPtr<T>::New(Args&&... args)
{
    return { SkrNew<T>(std::forward<Args>(args)...) };
}
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline UPtr<T> UPtr<T>::NewZeroed(Args&&... args)
{
    return { SkrNewZeroed<T>(std::forward<Args>(args)...) };
}

// compare
template <typename T>
inline bool operator==(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() == rhs.get();
}
template <typename T>
inline bool operator!=(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() != rhs.get();
}
template <typename T>
inline bool operator<(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() < rhs.get();
}
template <typename T>
inline bool operator>(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() > rhs.get();
}
template <typename T>
inline bool operator<=(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() <= rhs.get();
}
template <typename T>
inline bool operator>=(const UPtr<T>& lhs, const UPtr<T>& rhs)
{
    return lhs.get() >= rhs.get();
}

// compare with raw pointer (right)
template <typename T>
inline bool operator==(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() == rhs;
}
template <typename T>
inline bool operator!=(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() != rhs;
}
template <typename T>
inline bool operator<(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() < rhs;
}
template <typename T>
inline bool operator>(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() > rhs;
}
template <typename T>
inline bool operator<=(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() <= rhs;
}
template <typename T>
inline bool operator>=(const UPtr<T>& lhs, T* rhs)
{
    return lhs.get() >= rhs;
}

// compare with raw pointer (left)
template <typename T>
inline bool operator==(T* lhs, const UPtr<T>& rhs)
{
    return lhs == rhs.get();
}
template <typename T>
inline bool operator!=(T* lhs, const UPtr<T>& rhs)
{
    return lhs != rhs.get();
}
template <typename T>
inline bool operator<(T* lhs, const UPtr<T>& rhs)
{
    return lhs < rhs.get();
}
template <typename T>
inline bool operator>(T* lhs, const UPtr<T>& rhs)
{
    return lhs > rhs.get();
}
template <typename T>
inline bool operator<=(T* lhs, const UPtr<T>& rhs)
{
    return lhs <= rhs.get();
}
template <typename T>
inline bool operator>=(T* lhs, const UPtr<T>& rhs)
{
    return lhs >= rhs.get();
}

// compare with nullptr
template <typename T>
inline bool operator==(const UPtr<T>& lhs, std::nullptr_t)
{
    return lhs.get() == nullptr;
}
template <typename T>
inline bool operator!=(const UPtr<T>& lhs, std::nullptr_t)
{
    return lhs.get() != nullptr;
}
template <typename T>
inline bool operator==(std::nullptr_t, const UPtr<T>& rhs)
{
    return nullptr == rhs.get();
}
template <typename T>
inline bool operator!=(std::nullptr_t, const UPtr<T>& rhs)
{
    return nullptr != rhs.get();
}

// getter
template <typename T>
inline T* UPtr<T>::get() const
{
    return _ptr;
}

// empty
template <typename T>
inline bool UPtr<T>::is_empty() const
{
    return _ptr == nullptr;
}
template <typename T>
inline UPtr<T>::operator bool() const
{
    return !is_empty();
}

// ops
template <typename T>
inline void UPtr<T>::reset()
{
    if (_ptr)
    {
        SPDeleterTraits<T>::do_delete(_ptr);
        _ptr = nullptr;
    }
}
template <typename T>
inline void UPtr<T>::reset(T* ptr)
{
    if (_ptr != ptr)
    {
        if (_ptr)
        {
            SPDeleterTraits<T>::do_delete(_ptr);
        }
        _ptr = ptr;
    }
}
template <typename T>
template <SPConvertible<T> U>
inline void UPtr<T>::reset(U* ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (ptr)
    {
        reset(static_cast<T*>(ptr));
    }
    else
    {
        reset();
    }
}
template <typename T>
inline T* UPtr<T>::release()
{
    if (_ptr)
    {
        T* tmp = _ptr;
        _ptr   = nullptr;
        return tmp;
    }
    return nullptr;
}
template <typename T>
inline void UPtr<T>::swap(UPtr& rhs)
{
    if (this != &rhs)
    {
        T* tmp   = _ptr;
        _ptr     = rhs._ptr;
        rhs._ptr = tmp;
    }
}

// pointer behaviour
template <typename T>
inline T* UPtr<T>::operator->() const
{
    return _ptr;
}
template <typename T>
inline T& UPtr<T>::operator*() const
{
    return *_ptr;
}

// skr hash
template <typename T>
inline size_t UPtr<T>::_skr_hash(const UPtr& obj)
{
    return skr::Hash<T*>()(obj._ptr);
}
template <typename T>
inline size_t UPtr<T>::_skr_hash(T* ptr)
{
    return skr::Hash<T*>()(ptr);
}
} // namespace skr

// impl SP
namespace skr
{
// helper
template <typename T>
inline void SP<T>::_release()
{
    SKR_ASSERT(_ptr != nullptr);
    if (_counter->release())
    {
        SPDeleterTraits<T>::do_delete(_ptr);
    }
}

// ctor & dtor
template <typename T>
inline SP<T>::SP()
{
}
template <typename T>
inline SP<T>::SP(std::nullptr_t)
{
}
template <typename T>
inline SP<T>::SP(T* ptr)
    : _ptr(ptr)
{
    if (_ptr)
    {
        _counter = SkrNew<SPRefCounter>();
        _counter->add_ref();
    }
}
template <typename T>
inline SP<T>::SP(T* ptr, SPRefCounter* counter)
    : _ptr(ptr)
    , _counter(counter)
{
    if (_ptr)
    {
        _counter->add_ref();
    }
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>::SP(U* ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (ptr)
    {
        _ptr     = ptr;
        _counter = SkrNew<SPRefCounter>();
        _counter->add_ref();
    }
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>::SP(UPtr<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.release());
        _counter = SkrNew<SPRefCounter>();
        _counter->add_ref();
    }
}
template <typename T>
inline SP<T>::~SP()
{
    reset();
}

// copy & move
template <typename T>
inline SP<T>::SP(const SP& rhs)
    : _ptr(rhs._ptr)
    , _counter(rhs._counter)
{
    if (_ptr)
    {
        _counter->add_ref();
    }
}
template <typename T>
inline SP<T>::SP(SP&& rhs)
    : _ptr(rhs._ptr)
    , _counter(rhs._counter)
{
    rhs._ptr     = nullptr;
    rhs._counter = nullptr;
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>::SP(const SP<U>& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get());
        _counter = rhs.get_counter();
        _counter->add_ref();
    }
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>::SP(SP<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get());
        _counter = rhs.get_counter();
        _counter->add_ref();
        rhs.reset();
    }
}

// assign & move assign
template <typename T>
inline SP<T>& SP<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}
template <typename T>
inline SP<T>& SP<T>::operator=(T* ptr)
{
    reset(ptr);
    return *this;
}
template <typename T>
inline SP<T>& SP<T>::operator=(const SP& rhs)
{
    reset();
    if (this != &rhs)
    {
        _ptr     = rhs._ptr;
        _counter = rhs._counter;
        if (_counter)
        {
            _counter->add_ref();
        }
    }
    return *this;
}
template <typename T>
inline SP<T>& SP<T>::operator=(SP&& rhs)
{
    reset();
    if (this != &rhs)
    {
        _ptr         = rhs._ptr;
        _counter     = rhs._counter;
        rhs._ptr     = nullptr;
        rhs._counter = nullptr;
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>& SP<T>::operator=(U* ptr)
{
    if (ptr)
    {
        reset(static_cast<T*>(ptr));
    }
    else
    {
        reset();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>& SP<T>::operator=(const SP<U>& rhs)
{
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get());
        _counter = rhs.get_counter();
        _counter->add_ref();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>& SP<T>::operator=(SP<U>&& rhs)
{
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get());
        _counter = rhs.get_counter();
        _counter->add_ref();
        rhs.reset();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SP<T>& SP<T>::operator=(UPtr<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.release());
        _counter = SkrNew<SPRefCounter>();
        _counter->add_ref();
        rhs.reset();
    }
    return *this;
}

// factory
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline SP<T> SP<T>::New(Args&&... args)
{
    return { SkrNew<T>(std::forward<Args>(args)...) };
}
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline SP<T> SP<T>::NewZeroed(Args&&... args)
{
    return { SkrNewZeroed<T>(std::forward<Args>(args)...) };
}

// compare
template <typename T, typename U>
inline bool operator==(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() == rhs.get();
}
template <typename T, typename U>
inline bool operator!=(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() != rhs.get();
}
template <typename T, typename U>
inline bool operator<(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() < rhs.get();
}
template <typename T, typename U>
inline bool operator>(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() > rhs.get();
}
template <typename T, typename U>
inline bool operator<=(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() <= rhs.get();
}
template <typename T, typename U>
inline bool operator>=(const SP<T>& lhs, const SP<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get() == rhs.get()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SP compare with different counter, this is undefined behavior"
    );
    return lhs.get() >= rhs.get();
}

// compare with raw pointer (right)
template <typename T, typename U>
inline bool operator==(const SP<T>& lhs, U* rhs)
{
    return lhs.get() == rhs;
}
template <typename T, typename U>
inline bool operator!=(const SP<T>& lhs, U* rhs)
{
    return lhs.get() != rhs;
}
template <typename T, typename U>
inline bool operator<(const SP<T>& lhs, U* rhs)
{
    return lhs.get() < rhs;
}
template <typename T, typename U>
inline bool operator>(const SP<T>& lhs, U* rhs)
{
    return lhs.get() > rhs;
}
template <typename T, typename U>
inline bool operator<=(const SP<T>& lhs, U* rhs)
{
    return lhs.get() <= rhs;
}
template <typename T, typename U>
inline bool operator>=(const SP<T>& lhs, U* rhs)
{
    return lhs.get() >= rhs;
}

// compare with raw pointer (left)
template <typename T, typename U>
inline bool operator==(U* lhs, const SP<T>& rhs)
{
    return lhs == rhs.get();
}
template <typename T, typename U>
inline bool operator!=(U* lhs, const SP<T>& rhs)
{
    return lhs != rhs.get();
}
template <typename T, typename U>
inline bool operator<(U* lhs, const SP<T>& rhs)
{
    return lhs < rhs.get();
}
template <typename T, typename U>
inline bool operator>(U* lhs, const SP<T>& rhs)
{
    return lhs > rhs.get();
}
template <typename T, typename U>
inline bool operator<=(U* lhs, const SP<T>& rhs)
{
    return lhs <= rhs.get();
}
template <typename T, typename U>
inline bool operator>=(U* lhs, const SP<T>& rhs)
{
    return lhs >= rhs.get();
}

// compare with nullptr
template <typename T>
inline bool operator==(const SP<T>& lhs, std::nullptr_t)
{
    return lhs.get() == nullptr;
}
template <typename T>
inline bool operator!=(const SP<T>& lhs, std::nullptr_t)
{
    return lhs.get() != nullptr;
}
template <typename T>
inline bool operator==(std::nullptr_t, const SP<T>& rhs)
{
    return nullptr == rhs.get();
}
template <typename T>
inline bool operator!=(std::nullptr_t, const SP<T>& rhs)
{
    return nullptr != rhs.get();
}

// getter
template <typename T>
inline T* SP<T>::get() const
{
    return _ptr;
}

// count getter
template <typename T>
inline SPCounterType SP<T>::ref_count() const
{
    return _counter ? _counter->ref_count() : 0;
}
template <typename T>
inline SPCounterType SP<T>::ref_count_weak() const
{
    return _counter ? _counter->ref_count_weak() : 0;
}
template <typename T>
inline SPRefCounter* SP<T>::get_counter() const
{
    return _counter;
}

// empty
template <typename T>
inline bool SP<T>::is_empty() const
{
    return _ptr == nullptr;
}
template <typename T>
inline SP<T>::operator bool() const
{
    return !is_empty();
}

// ops
template <typename T>
inline void SP<T>::reset()
{
    if (_ptr)
    {
        _release();
        _ptr     = nullptr;
        _counter = nullptr;
    }
}
template <typename T>
inline void SP<T>::reset(T* ptr)
{
    if (_ptr != ptr)
    {
        reset();

        _ptr = ptr;
        if (_ptr)
        {
            _counter = SkrNew<SPRefCounter>();
            _counter->add_ref();
        }
    }
}
template <typename T>
template <SPConvertible<T> U>
inline void SP<T>::reset(U* ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (ptr)
    {
        reset(static_cast<T*>(ptr));
    }
    else
    {
        reset();
    }
}
template <typename T>
inline void SP<T>::swap(SP& rhs)
{
    if (this != &rhs)
    {
        T*            tmp_ptr     = _ptr;
        SPRefCounter* tmp_counter = _counter;
        _ptr                      = rhs._ptr;
        _counter                  = rhs._counter;
        rhs._ptr                  = tmp_ptr;
        rhs._counter              = tmp_counter;
    }
}

// pointer behaviour
template <typename T>
inline T* SP<T>::operator->() const
{
    return _ptr;
}
template <typename T>
inline T& SP<T>::operator*() const
{
    return *_ptr;
}

// cast
template <typename T>
template <typename U>
inline SP<U> SP<T>::cast_static() const
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<U>, "when use covariance, T must have virtual destructor for safe delete");
    if (_ptr)
    {
        return { static_cast<U*>(_ptr), _counter };
    }
    else
    {
        return nullptr;
    }
}

// skr hash
template <typename T>
inline size_t SP<T>::_skr_hash(const SP& obj)
{
    return ::skr::Hash<T*>()(obj._ptr);
}
template <typename T>
inline size_t SP<T>::_skr_hash(T* ptr)
{
    return ::skr::Hash<T*>()(ptr);
}
} // namespace skr

// impl SPWeak
namespace skr
{
// ctor & dtor
template <typename T>
inline SPWeak<T>::SPWeak()
{
}
template <typename T>
inline SPWeak<T>::SPWeak(std::nullptr_t)
{
}
template <typename T>
inline SPWeak<T>::SPWeak(T* ptr, SPRefCounter* counter)
    : _ptr(ptr)
    , _counter(counter)
{
    if (_ptr)
    {
        _counter->add_ref_weak();
    }
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>::SPWeak(const SP<U>& ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!ptr.is_empty())
    {
        _ptr     = static_cast<T*>(ptr.get());
        _counter = ptr.get_counter();
        _counter->add_ref_weak();
    }
}
template <typename T>
inline SPWeak<T>::~SPWeak()
{
    reset();
}

// copy & move
template <typename T>
inline SPWeak<T>::SPWeak(const SPWeak& rhs)
    : _ptr(rhs._ptr)
    , _counter(rhs._counter)
{
    if (_ptr)
    {
        _counter->add_ref_weak();
    }
}
template <typename T>
inline SPWeak<T>::SPWeak(SPWeak&& rhs)
    : _ptr(rhs._ptr)
    , _counter(rhs._counter)
{
    rhs._ptr     = nullptr;
    rhs._counter = nullptr;
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>::SPWeak(const SPWeak<U>& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get_unsafe());
        _counter = rhs.get_counter();
        _counter->add_ref_weak();
    }
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>::SPWeak(SPWeak<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get_unsafe());
        _counter = rhs.get_counter();
        _counter->add_ref_weak();
        rhs.reset();
    }
}

// assign & move assign
template <typename T>
inline SPWeak<T>& SPWeak<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}
template <typename T>
inline SPWeak<T>& SPWeak<T>::operator=(const SPWeak& rhs)
{
    if (this != &rhs)
    {
        reset();
        if (!rhs.is_empty())
        {
            _ptr     = rhs._ptr;
            _counter = rhs._counter;
            _counter->add_ref_weak();
        }
    }
    return *this;
}
template <typename T>
inline SPWeak<T>& SPWeak<T>::operator=(SPWeak&& rhs)
{
    if (this != &rhs)
    {
        reset();
        _ptr         = rhs._ptr;
        _counter     = rhs._counter;
        rhs._ptr     = nullptr;
        rhs._counter = nullptr;
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>& SPWeak<T>::operator=(const SPWeak<U>& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get_unsafe());
        _counter = rhs.get_counter();
        _counter->add_ref_weak();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>& SPWeak<T>::operator=(SPWeak<U>&& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get_unsafe());
        _counter = rhs.get_counter();
        _counter->add_ref_weak();
        rhs.reset();
    }
    return *this;
}
template <typename T>
template <SPConvertible<T> U>
inline SPWeak<T>& SPWeak<T>::operator=(const SP<U>& rhs)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    reset();
    if (!rhs.is_empty())
    {
        _ptr     = static_cast<T*>(rhs.get());
        _counter = rhs.get_counter();
        _counter->add_ref_weak();
    }
    return *this;
}

// compare
template <typename T, typename U>
inline bool operator==(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() == rhs.get_unsafe();
}
template <typename T, typename U>
inline bool operator!=(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() != rhs.get_unsafe();
}
template <typename T, typename U>
inline bool operator<(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() < rhs.get_unsafe();
}
template <typename T, typename U>
inline bool operator>(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() > rhs.get_unsafe();
}
template <typename T, typename U>
inline bool operator<=(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() <= rhs.get_unsafe();
}
template <typename T, typename U>
inline bool operator>=(const SPWeak<T>& lhs, const SPWeak<U>& rhs)
{
    SKR_ASSERT(
        (lhs.get_unsafe() == rhs.get_unsafe()) == (lhs.get_counter() == rhs.get_counter()) &&
        "SPWeak compare with different counter, this is undefined behavior"
    );
    return lhs.get_unsafe() >= rhs.get_unsafe();
}

// compare with nullptr
template <typename T>
inline bool operator==(const SPWeak<T>& lhs, std::nullptr_t)
{
    return lhs.get_unsafe() == nullptr;
}
template <typename T>
inline bool operator!=(const SPWeak<T>& lhs, std::nullptr_t)
{
    return lhs.get_unsafe() != nullptr;
}
template <typename T>
inline bool operator==(std::nullptr_t, const SPWeak<T>& rhs)
{
    return nullptr == rhs.get_unsafe();
}
template <typename T>
inline bool operator!=(std::nullptr_t, const SPWeak<T>& rhs)
{
    return nullptr != rhs.get_unsafe();
}

// unsafe getter
template <typename T>
inline T* SPWeak<T>::get_unsafe() const
{
    return _ptr;
}
template <typename T>
inline SPRefCounter* SPWeak<T>::get_counter() const
{
    return _counter;
}

// count getter
template <typename T>
inline SPCounterType SPWeak<T>::ref_count_weak() const
{
    return _counter ? _counter->ref_count_weak() : 0;
}

// lock
template <typename T>
inline SP<T> SPWeak<T>::lock() const
{
    if (_counter && _counter->try_lock_weak())
    {
        SP<T> result;
        result._ptr     = _ptr;
        result._counter = _counter;
        return result;
    }
    else
    {
        return nullptr;
    }
}

// empty
template <typename T>
inline bool SPWeak<T>::is_empty() const
{
    return _ptr == nullptr;
}
template <typename T>
inline bool SPWeak<T>::is_expired() const
{
    return !is_alive();
}
template <typename T>
inline bool SPWeak<T>::is_alive() const
{
    if (_ptr == nullptr) { return false; }
    return _counter->is_alive();
}
template <typename T>
inline SPWeak<T>::operator bool() const
{
    return is_alive();
}

// ops
template <typename T>
inline void SPWeak<T>::reset()
{
    if (_ptr)
    {
        _counter->release_weak();
        _ptr     = nullptr;
        _counter = nullptr;
    }
}
template <typename T>
template <SPConvertible<T> U>
inline void SPWeak<T>::reset(const SP<U>& ptr)
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<T>, "when use covariance, T must have virtual destructor for safe delete");
    if (ptr.get() != _ptr)
    {
        reset();
        if (!ptr.is_empty())
        {
            _ptr     = static_cast<T*>(ptr.get());
            _counter = ptr.get_counter();
            _counter->add_ref_weak();
        }
    }
}
template <typename T>
inline void SPWeak<T>::swap(SPWeak& rhs)
{
    if (this != &rhs)
    {
        T*            tmp_ptr     = _ptr;
        SPRefCounter* tmp_counter = _counter;
        _ptr                      = rhs._ptr;
        _counter                  = rhs._counter;
        rhs._ptr                  = tmp_ptr;
        rhs._counter              = tmp_counter;
    }
}

// cast
template <typename T>
template <typename U>
inline SPWeak<U> SPWeak<T>::cast_static() const
{
    static_assert(std::is_same_v<U, T> || std::has_virtual_destructor_v<U>, "when use covariance, T must have virtual destructor for safe delete");
    if (_ptr)
    {
        return { static_cast<U*>(_ptr), _counter };
    }
    else
    {
        return nullptr;
    }
}

// skr hash
template <typename T>
inline size_t SPWeak<T>::_skr_hash(const SPWeak& obj)
{
    return skr::Hash<T*>()(obj._ptr);
}

} // namespace skr
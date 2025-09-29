#pragma once
#include <SkrObject/fwd.hpp>
#include <SkrObject/object_ptr_basic.hpp>

// TODO. ObjPtr Resolve 时需要进行 RTTR Cast 来保证类型安全，原因如下：
//   1. Resolver 解析处理的对象类型是不确定的，虽然发生频率不高，但是无法通过编码规避
//   2. 当类型不匹配时应当告知用户，返回一个 Expected 或许较好？
//   3. Resolve 失败时不应该自动重置，对应行为应该由用户决定
//   4. ObjPtr 一般在反序列化时就会 Resolve 完毕，LazyResolve 发生频率较低
//   5. LazyResolve 只有给出标记时才会发生，默认不允许发生，这是为了避免误用和减轻心智负担
//   6. 为正常使用 Ptr 的场景增加 [[likely]] 以优化编译器的分支预测
//
// TODO. ObjPtrWeak Resolve 时候就不用检查，错误使用代价由用户承担，原因如下：
//   1. ObjPtrWeak 本身的传播较少，并且不允许 static_cast，强制 cast 代价由用户承担
//   2. ObjPtrWeak 的主要职责是探知对象是否存活，并不承担 cast 职责
//
// TODO. ObjPtr 与 ObjPtrLock 的转换必须保证指针的安全性，static cast 则由用户自行负责
// TODO. 所有的 cast_const 都需要进行转换类型正确性的检查，另外 RC 和 SP 的也需要补全
//
// TODO. 重要！ObjPtr 不应当支持 lazy resolve 功能，因为这对 UE 来说也是个 Editor Only 的
//       妥协功能，且会带来额外的复杂性和心智负担，更好的做法是使用 ObjPtrSoft 来表达这种需求，
//       在编码层面强制区分，避免误用
//
// TODO. size_t -> uint64_t 反正也不支持 32 位平台

// templated
namespace skr
{
template <typename T>
struct ObjPtr
{
    // ctor & dtor
    ObjPtr();
    ObjPtr(std::nullptr_t);
    ObjPtr(T* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtr(U* ptr);
    ObjPtr(ObjPtrBasic basic);
    ~ObjPtr();

    // copy & move
    ObjPtr(const ObjPtr& rhs);
    ObjPtr(ObjPtr&& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtr(const ObjPtr<U>& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtr(ObjPtr<U>&& rhs);

    // assign & move assign
    ObjPtr& operator=(const ObjPtr& rhs);
    ObjPtr& operator=(ObjPtr&& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtr& operator=(const ObjPtr<U>& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtr& operator=(ObjPtr<U>&& rhs);

    // pointer assign
    ObjPtr& operator=(std::nullptr_t);
    ObjPtr& operator=(T* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtr& operator=(U* ptr);

    // factory
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    static ObjPtr New(Args&&... args);

    // getter
    T* get() const;
    ObjPtrBasic& basic() &;
    const ObjPtrBasic& basic() const&;
    ObjPtrBasic&& basic() &&;

    // empty
    bool is_empty() const;
    operator bool() const;

    // resolve
    bool is_resolved() const;
    T* try_resolve();

    // ops
    void reset();
    void reset(T* ptr);
    template <concepts::ObjConvertible<T> U>
    void reset(U* ptr);
    void swap(ObjPtr& rhs);

    // pointer behavior
    T* operator->() const;
    T& operator*() const;

    // cast
    template <typename U>
    ObjPtr<U> cast_static() const;
    template <typename U>
    ObjPtr<U> cast_const() const;
    template <typename U>
    ObjPtr<U> cast_rttr() const;

    // skr hash
    static skr_hash _skr_hash(const ObjPtr& ptr);
    static skr_hash _skr_hash(T* ptr);

private:
    ObjPtrBasic _basic;
};

template <typename T>
struct ObjPtrWeak
{
    // ctor & dtor
    ObjPtrWeak();
    ObjPtrWeak(std::nullptr_t);
    ObjPtrWeak(T* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtrWeak(U* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtrWeak(const ObjPtr<U>& ptr);
    ~ObjPtrWeak();

    // copy & move
    ObjPtrWeak(const ObjPtrWeak& rhs);
    ObjPtrWeak(ObjPtrWeak&& rhs);

    // assign & move assign
    ObjPtrWeak& operator=(const ObjPtrWeak& rhs);
    ObjPtrWeak& operator=(ObjPtrWeak&& rhs);

    // getter
    ObjPtrWeakBasic& basic() &;
    const ObjPtrWeakBasic& basic() const&;
    ObjPtrWeakBasic&& basic() &&;

    // resolve
    T* resolve() const;

    // empty
    bool is_empty() const;
    operator bool() const;

    // ops
    void reset();
    void reset(T* ptr);
    template <concepts::ObjConvertible<T> U>
    void reset(U* ptr);
    template <concepts::ObjConvertible<T> U>
    void reset(const ObjPtr<U>& ptr);
    void swap(ObjPtrWeak& rhs);

    // lock
    ObjPtrLock<T> lock() const;

    // skr hash
    static skr_hash _skr_hash(const ObjPtrWeak& obj);

private:
    ObjPtrWeakBasic _basic;
};

template <typename T>
struct ObjPtrLock
{
    // ctor & dtor
    ObjPtrLock();
    ObjPtrLock(std::nullptr_t);
    ObjPtrLock(T* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock(U* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock(const ObjPtr<U>& ptr);
    ~ObjPtrLock();

    // copy & move
    ObjPtrLock(const ObjPtrLock& rhs);
    ObjPtrLock(ObjPtrLock&& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock(const ObjPtrLock<U>& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock(ObjPtrLock<U>&& rhs);

    // assign & move assign
    ObjPtrLock& operator=(const ObjPtrLock& rhs);
    ObjPtrLock& operator=(ObjPtrLock&& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock& operator=(const ObjPtrLock<U>& rhs);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock& operator=(ObjPtrLock<U>&& rhs);

    // pointer assign
    ObjPtrLock& operator=(std::nullptr_t);
    ObjPtrLock& operator=(T* ptr);
    template <concepts::ObjConvertible<T> U>
    ObjPtrLock& operator=(U* ptr);

    // getter
    T* get() const;
    ObjPtrLockBasic& get_basic() &;
    const ObjPtrLockBasic& get_basic() const&;
    ObjPtrLockBasic&& get_basic() &&;

    // empty
    bool is_empty() const;
    operator bool() const;

    // ops
    void reset();
    void reset(T* ptr);
    template <concepts::ObjConvertible<T> U>
    void reset(U* ptr);
    void swap(ObjPtrLock& rhs);

    // pointer behavior
    T* operator->() const;
    T& operator*() const;

    // cast
    template <typename U>
    ObjPtrLock<U> cast_static() const;
    template <typename U>
    ObjPtrLock<U> cast_const() const;
    template <typename U>
    ObjPtrLock<U> cast_rttr() const;

    // skr hash
    static skr_hash _skr_hash(const ObjPtrLock& obj);
    static skr_hash _skr_hash(T* ptr);

private:
    ObjPtrLockBasic _basic;
};
} // namespace skr

// impl ObjPtr
namespace skr
{
// ctor & dtor
template <typename T>
inline ObjPtr<T>::ObjPtr()
{
}
template <typename T>
inline ObjPtr<T>::ObjPtr(std::nullptr_t)
{
}
template <typename T>
inline ObjPtr<T>::ObjPtr(T* ptr)
    : _basic(ptr)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>::ObjPtr(U* ptr)
    : _basic(static_cast<T*>(ptr))
{
}
template <typename T>
inline ObjPtr<T>::ObjPtr(ObjPtrBasic basic)
    : _basic(basic)
{
}
template <typename T>
inline ObjPtr<T>::~ObjPtr()
{
}

// copy & move
template <typename T>
inline ObjPtr<T>::ObjPtr(const ObjPtr& rhs)
    : _basic(rhs._basic)
{
}
template <typename T>
inline ObjPtr<T>::ObjPtr(ObjPtr&& rhs)
    : _basic(std::move(rhs._basic))
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>::ObjPtr(const ObjPtr<U>& rhs)
    : _basic(rhs.basic())
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>::ObjPtr(ObjPtr<U>&& rhs)
    : _basic(std::move(rhs.basic()))
{
}

// assign & move assign
template <typename T>
inline ObjPtr<T>& ObjPtr<T>::operator=(const ObjPtr& rhs)
{
    _basic = rhs._basic;
    return *this;
}
template <typename T>
inline ObjPtr<T>& ObjPtr<T>::operator=(ObjPtr&& rhs)
{
    _basic = std::move(rhs._basic);
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>& ObjPtr<T>::operator=(const ObjPtr<U>& rhs)
{
    _basic = rhs.basic();
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>& ObjPtr<T>::operator=(ObjPtr<U>&& rhs)
{
    _basic = std::move(rhs.basic());
    return *this;
}

// pointer assign
template <typename T>
inline ObjPtr<T>& ObjPtr<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}
template <typename T>
inline ObjPtr<T>& ObjPtr<T>::operator=(T* ptr)
{
    reset(ptr);
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtr<T>& ObjPtr<T>::operator=(U* ptr)
{
    reset(ptr);
    return *this;
}

// factory
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline ObjPtr<T> ObjPtr<T>::New(Args&&... args)
{
    T* obj = SkrNew<T>(std::forward<Args>(args)...);
    return { obj };
}

// getter
template <typename T>
inline T* ObjPtr<T>::get() const
{
    return static_cast<T*>(_basic.get_object());
}
template <typename T>
inline ObjPtrBasic& ObjPtr<T>::basic() &
{
    return _basic;
}
template <typename T>
inline const ObjPtrBasic& ObjPtr<T>::basic() const&
{
    return _basic;
}
template <typename T>
inline ObjPtrBasic&& ObjPtr<T>::basic() &&
{
    return std::move(_basic);
}

// empty
template <typename T>
inline bool ObjPtr<T>::is_empty() const
{
    return _basic.is_empty();
}
template <typename T>
inline ObjPtr<T>::operator bool() const
{
    return !is_empty();
}

// resolve
template <typename T>
inline bool ObjPtr<T>::is_resolved() const
{
    return _basic.is_resolved();
}
template <typename T>
inline T* ObjPtr<T>::try_resolve()
{
    if (is_resolved())
    {
        return get();
    }
    else
    {
        Object* obj = _basic.try_resolve();
        if (obj && obj->template rttr_is<T>())
        {
            return static_cast<T*>(obj);
        }
        else
        {
            _basic = nullptr;
            return nullptr;
        }
    }
}

// ops
template <typename T>
inline void ObjPtr<T>::reset()
{
    _basic.reset();
}
template <typename T>
inline void ObjPtr<T>::reset(T* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline void ObjPtr<T>::reset(U* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
inline void ObjPtr<T>::swap(ObjPtr& rhs)
{
    std::swap(_basic, rhs._basic);
}

// pointer behavior
template <typename T>
inline T* ObjPtr<T>::operator->() const
{
    SKR_ASSERT(is_resolved());
    return get();
}
template <typename T>
inline T& ObjPtr<T>::operator*() const
{
    SKR_ASSERT(is_resolved());
    return *get();
}

// cast
template <typename T>
template <typename U>
inline ObjPtr<U> ObjPtr<T>::cast_static() const
{
    if (is_empty())
    {
        return { nullptr };
    }
    else
    {
        return { _basic };
    }
}
template <typename T>
template <typename U>
inline ObjPtr<U> ObjPtr<T>::cast_const() const
{
    if (is_empty())
    {
        return { nullptr };
    }
    else
    {
        return { _basic };
    }
}
template <typename T>
template <typename U>
inline ObjPtr<U> ObjPtr<T>::cast_rttr() const
{
    if (is_empty())
    {
        return { nullptr };
    }
    else if (is_resolved())
    {
        if (get().template rttr_is<U>())
        {
            return { _basic };
        }
        else
        {
            return { nullptr };
        }
    }
    else
    {
        return { _basic };
    }
}

// skr hash
template <typename T>
inline skr_hash ObjPtr<T>::_skr_hash(const ObjPtr& obj)
{
    return ObjPtrBasic::_skr_hash(obj._basic);
}
template <typename T>
inline skr_hash ObjPtr<T>::_skr_hash(T* ptr)
{
    return (skr_hash) reinterpret_cast<intptr_t>(ptr);
}

} // namespace skr

// impl ObjPtrWeak
namespace skr
{
// ctor & dtor
template <typename T>
inline ObjPtrWeak<T>::ObjPtrWeak()
{
}
template <typename T>
inline ObjPtrWeak<T>::ObjPtrWeak(std::nullptr_t)
{
}
template <typename T>
inline ObjPtrWeak<T>::ObjPtrWeak(T* ptr)
    : _basic(ptr)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrWeak<T>::ObjPtrWeak(U* ptr)
    : _basic(ptr)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrWeak<T>::ObjPtrWeak(const ObjPtr<U>& ptr)
    : _basic(ptr.get())
{
}
template <typename T>
inline ObjPtrWeak<T>::~ObjPtrWeak()
{
}

// copy & move
template <typename T>
inline ObjPtrWeak<T>::ObjPtrWeak(const ObjPtrWeak& rhs)
    : _basic(rhs._basic)
{
}
template <typename T>
inline ObjPtrWeak<T>::ObjPtrWeak(ObjPtrWeak&& rhs)
    : _basic(std::move(rhs._basic))
{
}

// assign & move assign
template <typename T>
inline ObjPtrWeak<T>& ObjPtrWeak<T>::operator=(const ObjPtrWeak& rhs)
{
    _basic = rhs._basic;
    return *this;
}
template <typename T>
inline ObjPtrWeak<T>& ObjPtrWeak<T>::operator=(ObjPtrWeak&& rhs)
{
    _basic = std::move(rhs._basic);
    return *this;
}

// getter
template <typename T>
inline ObjPtrWeakBasic& ObjPtrWeak<T>::basic() &
{
    return _basic;
}
template <typename T>
inline const ObjPtrWeakBasic& ObjPtrWeak<T>::basic() const&
{
    return _basic;
}
template <typename T>
inline ObjPtrWeakBasic&& ObjPtrWeak<T>::basic() &&
{
    return std::move(_basic);
}

// resolve
template <typename T>
inline T* ObjPtrWeak<T>::resolve() const
{
    T* obj = _basic.resolve();
    return obj ? static_cast<T*>(obj) : nullptr;
}

// empty
template <typename T>
inline bool ObjPtrWeak<T>::is_empty() const
{
    return _basic.is_empty();
}
template <typename T>
inline ObjPtrWeak<T>::operator bool() const
{
    return !is_empty();
}

// ops
template <typename T>
inline void ObjPtrWeak<T>::reset()
{
    _basic.reset();
}
template <typename T>
inline void ObjPtrWeak<T>::reset(T* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline void ObjPtrWeak<T>::reset(U* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline void ObjPtrWeak<T>::reset(const ObjPtr<U>& ptr)
{
    if (ptr.is_resolved() && !ptr.is_empty())
    {
        _basic.reset(ptr.get());
    }
    else
    {
        _basic.reset();
    }
}
template <typename T>
inline void ObjPtrWeak<T>::swap(ObjPtrWeak& rhs)
{
    std::swap(_basic, rhs._basic);
}

// lock
template <typename T>
inline ObjPtrLock<T> ObjPtrWeak<T>::lock() const
{
    Object* obj = _basic.resolve();
    if (obj)
    {
        return { static_cast<T*>(obj) };
    }
    else
    {
        return { nullptr };
    }
}

// skr hash
template <typename T>
inline skr_hash ObjPtrWeak<T>::_skr_hash(const ObjPtrWeak& obj)
{
    return ObjPtrWeakBasic::_skr_hash(obj._basic);
}
} // namespace skr

// impl ObjPtrLock
namespace skr
{
// ctor & dtor
template <typename T>
inline ObjPtrLock<T>::ObjPtrLock()
{
}
template <typename T>
inline ObjPtrLock<T>::ObjPtrLock(std::nullptr_t)
{
}
template <typename T>
inline ObjPtrLock<T>::ObjPtrLock(T* ptr)
    : _basic(ptr)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>::ObjPtrLock(U* ptr)
    : _basic(ptr)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>::ObjPtrLock(const ObjPtr<U>& ptr)
{
    if (ptr.is_resolved() && !ptr.is_empty())
    {
        _basic.reset(ptr.get());
    }
}
template <typename T>
inline ObjPtrLock<T>::~ObjPtrLock()
{
}

// copy & move
template <typename T>
inline ObjPtrLock<T>::ObjPtrLock(const ObjPtrLock& rhs)
    : _basic(rhs._basic)
{
}
template <typename T>
inline ObjPtrLock<T>::ObjPtrLock(ObjPtrLock&& rhs)
    : _basic(std::move(rhs._basic))
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>::ObjPtrLock(const ObjPtrLock<U>& rhs)
    : _basic(rhs._basic)
{
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>::ObjPtrLock(ObjPtrLock<U>&& rhs)
    : _basic(std::move(rhs._basic))
{
}

// assign & move assign
template <typename T>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(const ObjPtrLock& rhs)
{
    _basic = rhs._basic;
    return *this;
}
template <typename T>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(ObjPtrLock&& rhs)
{
    _basic = std::move(rhs._basic);
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(const ObjPtrLock<U>& rhs)
{
    _basic = rhs._basic;
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(ObjPtrLock<U>&& rhs)
{
    _basic = std::move(rhs._basic);
    return *this;
}

// pointer assign
template <typename T>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}
template <typename T>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(T* ptr)
{
    reset(ptr);
    return *this;
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline ObjPtrLock<T>& ObjPtrLock<T>::operator=(U* ptr)
{
    reset(ptr);
    return *this;
}

// getter
template <typename T>
inline T* ObjPtrLock<T>::get() const
{
    return static_cast<T*>(_basic.get());
}
template <typename T>
inline ObjPtrLockBasic& ObjPtrLock<T>::get_basic() &
{
    return _basic;
}
template <typename T>
inline const ObjPtrLockBasic& ObjPtrLock<T>::get_basic() const&
{
    return _basic;
}
template <typename T>
inline ObjPtrLockBasic&& ObjPtrLock<T>::get_basic() &&
{
    return std::move(_basic);
}

// empty
template <typename T>
inline bool ObjPtrLock<T>::is_empty() const
{
    return _basic.is_empty();
}
template <typename T>
inline ObjPtrLock<T>::operator bool() const
{
    return !is_empty();
}

// ops
template <typename T>
inline void ObjPtrLock<T>::reset()
{
    _basic.reset();
}
template <typename T>
inline void ObjPtrLock<T>::reset(T* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
template <concepts::ObjConvertible<T> U>
inline void ObjPtrLock<T>::reset(U* ptr)
{
    _basic.reset(ptr);
}
template <typename T>
inline void ObjPtrLock<T>::swap(ObjPtrLock& rhs)
{
    return std::swap(_basic, rhs._basic);
}

// pointer behavior
template <typename T>
inline T* ObjPtrLock<T>::operator->() const
{
    SKR_ASSERT(!is_empty());
    return get();
}
template <typename T>
inline T& ObjPtrLock<T>::operator*() const
{
    SKR_ASSUME(!is_empty());
    return *get();
}

// cast
template <typename T>
template <typename U>
inline ObjPtrLock<U> ObjPtrLock<T>::cast_static() const
{
    if (is_empty())
    {
        return { nullptr };
    }
    else
    {
        return { _basic };
    }
}
template <typename T>
template <typename U>
inline ObjPtrLock<U> ObjPtrLock<T>::cast_const() const
{
    return { _basic };
}
template <typename T>
template <typename U>
inline ObjPtrLock<U> ObjPtrLock<T>::cast_rttr() const
{
    if (!is_empty() && get()->template rttr_is<U>())
    {
        return { _basic };
    }
    else
    {
        return { nullptr };
    }
}

// skr hash
template <typename T>
inline skr_hash ObjPtrLock<T>::_skr_hash(const ObjPtrLock& obj)
{
    return ObjPtrLockBasic::_skr_hash(obj._basic);
}
template <typename T>
inline skr_hash ObjPtrLock<T>::_skr_hash(T* ptr)
{
    return ObjPtrLockBasic::_skr_hash(ptr);
}

} // namespace skr
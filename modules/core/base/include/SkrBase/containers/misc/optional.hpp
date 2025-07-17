#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/memory/memory_ops.hpp"

namespace skr::container
{
struct Nullopt {
};

template <typename T>
struct Optional {
    using DataType = T;
    static_assert(!std::is_same_v<T, void>, "Optional<void> is not allowed");

    // ctor & dtor
    Optional();
    Optional(Nullopt);
    Optional(const T& value);
    Optional(T&& value);
    template <std::convertible_to<T> U>
    Optional(const U& value);
    ~Optional();

    // copy & move
    Optional(const Optional& other);
    Optional(Optional&& other);
    template <typename U>
    Optional(const Optional<U>& other);
    template <typename U>
    Optional(Optional<U>&& other);

    // assign & move assign
    Optional& operator=(const Optional& other);
    Optional& operator=(Optional&& other);
    template <typename U = T>
    Optional& operator=(const U& value);
    template <typename U = T>
    Optional& operator=(U&& value);
    template <typename U>
    Optional& operator=(const Optional<U>& other);
    template <typename U>
    Optional& operator=(Optional<U>&& other);

    // value checker
    inline operator bool() const { return _has_value; }
    bool   has_value() const;

    // getter
    T&        operator*();
    const T&  operator*() const;
    T*        operator->();
    const T*  operator->() const;
    T&        value() &;
    const T&  value() const&;
    T&&       value() &&;
    const T&& value() const&&;
    template <typename U = T>
    T value_or(U&& default_value) const&;
    template <typename U = T>
    T value_or(U&& default_value) &&;

    // modifier
    void reset();
    template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
    void emplace(Args&&... args);

private:
    // helper
    T*       _data_ptr();
    const T* _data_ptr() const;

private:
    bool           _has_value   = false;
    Placeholder<T> _placeholder = {};
};
} // namespace skr::container

namespace skr::container
{
// ctor & dtor
template <typename T>
inline Optional<T>::Optional() = default;
template <typename T>
inline Optional<T>::Optional(Nullopt) {}
template <typename T>
inline Optional<T>::Optional(const T& value)
    : _has_value(true)
{
    ::skr::memory::copy(_data_ptr(), &value);
}
template <typename T>
inline Optional<T>::Optional(T&& value)
    : _has_value(true)
{
    ::skr::memory::move(_data_ptr(), &value);
}
template <typename T>
template <std::convertible_to<T> U>
inline Optional<T>::Optional(const U& value)
    : _has_value(true)
{
    new (_data_ptr()) T(value);
}
template <typename T>
inline Optional<T>::~Optional()
{
    if (_has_value)
    {
        reset();
    }
}

// copy & move
template <typename T>
inline Optional<T>::Optional(const Optional& other)
    : _has_value(other._has_value)
{
    if (other._has_value)
    {
        ::skr::memory::copy(_data_ptr(), other._data_ptr());
    }
}
template <typename T>
inline Optional<T>::Optional(Optional&& other)
    : _has_value(other._has_value)
{
    if (other._has_value)
    {
        ::skr::memory::move(_data_ptr(), other._data_ptr());
    }
    other.reset();
}
template <typename T>
template <typename U>
inline Optional<T>::Optional(const Optional<U>& other)
    : _has_value(other.has_value())
{
    if (other.has_value())
    {
        ::skr::memory::copy(_data_ptr(), &other.value());
    }
}
template <typename T>
template <typename U>
inline Optional<T>::Optional(Optional<U>&& other)
    : _has_value(other.has_value())
{
    if (other.has_value())
    {
        ::skr::memory::move(_data_ptr(), &other.value());
    }
    other.reset();
}

// assign & move assign
template <typename T>
inline Optional<T>& Optional<T>::operator=(const Optional& other)
{
    if (other._has_value)
    {
        if (_has_value)
        {
            ::skr::memory::assign(_data_ptr(), other._data_ptr());
        }
        else
        {
            ::skr::memory::copy(_data_ptr(), other._data_ptr());
            _has_value = true;
        }
    }
    else
    {
        reset();
    }
    return *this;
}
template <typename T>
inline Optional<T>& Optional<T>::operator=(Optional&& other)
{
    if (other.has_value())
    {
        if (_has_value)
        {
            ::skr::memory::move_assign(_data_ptr(), other._data_ptr());
        }
        else
        {
            ::skr::memory::move(_data_ptr(), other._data_ptr());
            _has_value = true;
        }
    }
    else
    {
        reset();
    }
    other.reset();
    return *this;
}
template <typename T>
template <typename U>
inline Optional<T>& Optional<T>::operator=(const U& value)
{
    if (_has_value)
    {
        ::skr::memory::assign(_data_ptr(), &value);
    }
    else
    {
        ::skr::memory::copy(_data_ptr(), &value);
        _has_value = true;
    }
    return *this;
}
template <typename T>
template <typename U>
inline Optional<T>& Optional<T>::operator=(U&& value)
{
    if (_has_value)
    {
        ::skr::memory::move_assign(_data_ptr(), &value);
    }
    else
    {
        ::skr::memory::move(_data_ptr(), &value);
        _has_value = true;
    }
    return *this;
}
template <typename T>
template <typename U>
inline Optional<T>& Optional<T>::operator=(const Optional<U>& other)
{
    if (other.has_value())
    {
        if (_has_value)
        {
            ::skr::memory::assign(_data_ptr(), &other.value());
        }
        else
        {
            ::skr::memory::copy(_data_ptr(), &other.value());
            _has_value = true;
        }
    }
    else
    {
        reset();
    }
    return *this;
}
template <typename T>
template <typename U>
inline Optional<T>& Optional<T>::operator=(Optional<U>&& other)
{
    if (other.has_value())
    {
        if (_has_value)
        {
            ::skr::memory::move_assign(_data_ptr(), &other.value());
        }
        else
        {
            ::skr::memory::move(_data_ptr(), &other.value());
            _has_value = true;
        }
    }
    else
    {
        reset();
    }
    return *this;
}

// value checker
template <typename T>
inline bool Optional<T>::has_value() const
{
    return _has_value;
}

// getter
template <typename T>
inline T& Optional<T>::operator*()
{
    return *_data_ptr();
}
template <typename T>
inline const T& Optional<T>::operator*() const
{
    return *_data_ptr();
}
template <typename T>
T* Optional<T>::operator->()
{
    return _data_ptr();
}
template <typename T>
const T* Optional<T>::operator->() const
{
    return _data_ptr();
}
template <typename T>
inline T& Optional<T>::value() &
{
    return *_data_ptr();
}
template <typename T>
inline const T& Optional<T>::value() const&
{
    return *_data_ptr();
}
template <typename T>
inline T&& Optional<T>::value() &&
{
    return std::move(*_data_ptr());
}
template <typename T>
inline const T&& Optional<T>::value() const&&
{
    return std::move(*_data_ptr());
}
template <typename T>
template <typename U>
inline T Optional<T>::value_or(U&& default_value) const&
{
    if (_has_value)
    {
        return *_data_ptr();
    }
    else
    {
        return static_cast<T>(std::forward<U>(default_value));
    }
}
template <typename T>
template <typename U>
inline T Optional<T>::value_or(U&& default_value) &&
{
    if (_has_value)
    {
        return std::move(*_data_ptr());
    }
    else
    {
        return static_cast<T>(std::forward<U>(default_value));
    }
}

// modifier
template <typename T>
inline void Optional<T>::reset()
{
    if (_has_value)
    {
        ::skr::memory::destruct(_data_ptr());
        _has_value = false;
    }
}
template <typename T>
template <typename... Args>
requires(std::is_constructible_v<T, Args...>)
inline void Optional<T>::emplace(Args&&... args)
{
    reset();
    new (_data_ptr()) T(std::forward<Args>(args)...);
    _has_value = true;
}

// helper
template <typename T>
inline T* Optional<T>::_data_ptr()
{
    return _placeholder.data_typed();
}
template <typename T>
inline const T* Optional<T>::_data_ptr() const
{
    return _placeholder.data_typed();
}
} // namespace skr::container
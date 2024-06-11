#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"

namespace skr {

template <typename T>
struct ExpectedValue 
{ 
    using type = T; 
    static constexpr auto is_void = false;
};

template <>
struct ExpectedValue<void> 
{ 
    using type = bool;
    static constexpr auto is_void = true;
};

template <typename E, typename T = void>
struct SKR_STATIC_API Expected
{
public:
    using ValueType = typename ExpectedValue<T>::type;
    static_assert(!std::is_same_v<E, T>, "E and T cannot be the same type");

    Expected() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void);
    Expected(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    Expected(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    Expected(const E& error) SKR_NOEXCEPT;
    Expected(E&& error) SKR_NOEXCEPT;
    Expected(Expected&& other) SKR_NOEXCEPT;
    Expected(const Expected& other) = delete;

    Expected& operator=(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    Expected& operator=(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    Expected& operator=(const E& error) SKR_NOEXCEPT;
    Expected& operator=(E&& error) SKR_NOEXCEPT;
    Expected& operator=(Expected&& other) SKR_NOEXCEPT;
    Expected& operator=(const Expected& other) = delete;

    ~Expected() SKR_NOEXCEPT;

    bool has_value() const SKR_NOEXCEPT { return _hasValue; }
    bool has_error() const SKR_NOEXCEPT { return !_hasValue; }

    const ValueType& value() const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    ValueType& value() SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    void value() const SKR_NOEXCEPT requires(ExpectedValue<T>::is_void);
    void value() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void);
    const E& error() const SKR_NOEXCEPT;

    template <typename V>
    bool operator==(const V& v) const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
    {
        return has_value() && (value() == v);
    }

    bool operator==(const E& e) const SKR_NOEXCEPT
    {
        return has_error() && (error() == e);
    }

    bool operator==(const Expected& other) const SKR_NOEXCEPT
    {
        if (has_value() && other.has_value())
            return _value == other._value;
        else if (has_error() && other.has_error())
            return error() == other.error();
        return false;
    }

    template <typename F>
    void error_then(F&& f) SKR_NOEXCEPT
    {
        if (!_hasValue && _unhandled)
        {
            f(error());
            _unhandled = false;
        }
    }

    template <typename F>
    void and_then(F&& f) SKR_NOEXCEPT
    {
        if (_hasValue)
        {
            if constexpr (ExpectedValue<T>::is_void)
                f();
            else
                f(value());
        }
    }

protected:
    enum PanicReasion
    {
        ErrorButNotValue,
        ValueButNotError,
        ErrorNotHandled
    };
    void CondPanic(PanicReasion reason) const;

    union {
        E _error;
        ValueType _value;
    };
    bool _hasValue;
    bool _unhandled;
};

template <typename E, typename T>
Expected<E, T>::Expected() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
    : _value(true), _hasValue(true), _unhandled(false)
{

}

template <typename E, typename T>
Expected<E, T>::Expected(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
    : _value(value), _hasValue(true), _unhandled(false)
{

}

template <typename E, typename T>
Expected<E, T>::Expected(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
    : _value(std::move(value)), _hasValue(true), _unhandled(false)
{

}

template <typename E, typename T>
Expected<E, T>::Expected(const E& error) SKR_NOEXCEPT 
    : _error(error), _hasValue(false), _unhandled(true)
{

}

template <typename E, typename T>
Expected<E, T>::Expected(E&& error) SKR_NOEXCEPT 
    : _error(std::move(error)), _hasValue(false), _unhandled(true)
{

}
template <typename E, typename T>
Expected<E, T>::Expected(Expected&& other) SKR_NOEXCEPT
    : _hasValue(other._hasValue), _unhandled(other._unhandled)
{
    other._unhandled = false;
    if (_hasValue)
        new (&_value) ValueType(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));
}

template <typename E, typename T>
Expected<E, T>& Expected<E, T>::operator=(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    _hasValue = true;
    _unhandled = false;
    _value = value;
    return *this;
}

template <typename E, typename T>
Expected<E, T>& Expected<E, T>::operator=(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    _hasValue = true;
    _unhandled = false;
    _value = std::move(value);
    return *this;
}

template <typename E, typename T>
Expected<E, T>& Expected<E, T>::operator=(const E& error) SKR_NOEXCEPT
{
    _hasValue = false;
    _unhandled = true;
    _error = error;
    return *this;
}

template <typename E, typename T>
Expected<E, T>& Expected<E, T>::operator=(E&& error) SKR_NOEXCEPT
{
    _hasValue = false;
    _unhandled = true;
    _error = std::move(error);
    return *this;
}

template <typename E, typename T>
Expected<E, T>& Expected<E, T>::operator=(Expected&& other) SKR_NOEXCEPT
{
    if (_hasValue)
        _value.~ValueType();
    else
        _error.~E();

    other._unhandled = false;
    _hasValue = other._hasValue;
    if (_hasValue)
        new (&_value) ValueType(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));

    return *this;
}

template <typename E, typename T>
Expected<E, T>::~Expected() SKR_NOEXCEPT
{
    CondPanic(ErrorNotHandled);

    if (_hasValue)
        _value.~ValueType();
    else
        _error.~E();
}

template <typename E, typename T>
const typename Expected<E, T>::ValueType& Expected<E, T>::value() const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    CondPanic(ErrorButNotValue);
    return _value;
}

template <typename E, typename T>
typename Expected<E, T>::ValueType& Expected<E, T>::value() SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    CondPanic(ErrorButNotValue);
    return _value;
}

template <typename E, typename T>
void Expected<E, T>::value() const SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
{
    CondPanic(ErrorButNotValue);
}

template <typename E, typename T>
void Expected<E, T>::value() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
{
    CondPanic(ErrorButNotValue);
}

template <typename E, typename T>
const E& Expected<E, T>::error() const SKR_NOEXCEPT
{
    CondPanic(ValueButNotError);
    return _error;
}

template <typename E, typename T>
void Expected<E, T>::CondPanic(PanicReasion reason) const
{
    switch (reason)
    {
    case ErrorButNotValue:
        SKR_ASSERT(_hasValue && "Expect<E, T> contains an error but not value!");
        break;
    case ValueButNotError:
        SKR_ASSERT(!_hasValue && "Expect<E, T> contains a value but not error!");
        break;
    case ErrorNotHandled:
        SKR_ASSERT((_hasValue || !_unhandled) && "Expect<E, T> contains an error but not handled!");
        break;
    default:
        SKR_ASSERT(false && "Unknown panic reason!");
        break;
    }
}

}
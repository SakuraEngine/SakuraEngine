#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"

namespace skr {

template <typename T, typename E>
struct SKR_STATIC_API Expected
{
public:
    Expected() : _hasValue(false) {}
    Expected(const T& value) : _value(value), _hasValue(true) {}
    Expected(T&& value) : _value(std::move(value)), _hasValue(true) {}
    Expected(const E& error) : _error(error), _hasValue(false) {}
    Expected(E&& error) : _error(std::move(error)), _hasValue(false) {}

    Expected(const Expected& other);
    Expected(Expected&& other);
    ~Expected();

    Expected& operator=(const Expected& other);
    Expected& operator=(Expected&& other);

    bool hasValue() const { return _hasValue; }
    bool hasError() const { return !_hasValue; }

    const T& value() const
    {
        SKR_ASSERT(_hasValue);
        return _value;
    }

    T& value()
    {
        SKR_ASSERT(_hasValue);
        return _value;
    }

    const E& error() const
    {
        SKR_ASSERT(!_hasValue);
        return _error;
    }

protected:
    union {
        T _value;
        E _error;
    };
    bool _hasValue;
};

template <typename T, typename E>
Expected<T, E>::Expected(const Expected& other) : _hasValue(other._hasValue)
{
    if (_hasValue)
        new (&_value) T(other._value);
    else
        new (&_error) E(other._error);
}

template <typename T, typename E>
Expected<T, E>::Expected(Expected&& other) : _hasValue(other._hasValue)
{
    if (_hasValue)
        new (&_value) T(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));
}

template <typename T, typename E>
Expected<T, E>::~Expected()
{
    if (_hasValue)
        _value.~T();
    else
        _error.~E();
}

template <typename T, typename E>
Expected<T, E>& Expected<T, E>::operator=(const Expected& other)
{
    if (_hasValue)
        _value.~T();
    else
        _error.~E();

    _hasValue = other._hasValue;
    if (_hasValue)
        new (&_value) T(other._value);
    else
        new (&_error) E(other._error);

    return *this;
}

template <typename T, typename E>
Expected<T, E>& Expected<T, E>::operator=(Expected&& other)
{
    if (_hasValue)
        _value.~T();
    else
        _error.~E();

    _hasValue = other._hasValue;
    if (_hasValue)
        new (&_value) T(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));

    return *this;
}

}
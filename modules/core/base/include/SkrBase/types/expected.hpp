#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrContainersDef/optional.hpp"

namespace skr
{

template <typename T>
struct ExpectedValue {
    using type                    = T;
    static constexpr auto is_void = false;
};

template <>
struct ExpectedValue<void> {
    using type                    = bool;
    static constexpr auto is_void = true;
};

template <typename E, typename T = void>
struct SKR_STATIC_API Expected {
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
    ValueType&       value() SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    void             value() const SKR_NOEXCEPT requires(ExpectedValue<T>::is_void);
    void             value() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void);
    const E&         error() const SKR_NOEXCEPT;

    void mark_handled();

    template <typename V>
    bool operator==(const V& v) const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void);
    bool operator==(const E& e) const SKR_NOEXCEPT;
    bool operator==(const Expected& other) const SKR_NOEXCEPT;

    template <typename F>
    Expected& error_then(F&& f) SKR_NOEXCEPT;
    template <typename F>
    Expected& and_then(F&& f) SKR_NOEXCEPT;

    Optional<E> take_error();

private:
    union
    {
        E         _error;
        ValueType _value;
    };
    bool _hasValue;
    bool _unhandled;
};

template <typename E, typename T>
inline Expected<E, T>::Expected() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
    : _value(true)
    , _hasValue(true)
    , _unhandled(false)
{
}

template <typename E, typename T>
inline Expected<E, T>::Expected(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
    : _value(value)
    , _hasValue(true)
    , _unhandled(false)
{
}

template <typename E, typename T>
inline Expected<E, T>::Expected(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
    : _value(std::move(value))
    , _hasValue(true)
    , _unhandled(false)
{
}

template <typename E, typename T>
inline Expected<E, T>::Expected(const E& error) SKR_NOEXCEPT
    : _error(error),
      _hasValue(false),
      _unhandled(true)
{
}

template <typename E, typename T>
inline Expected<E, T>::Expected(E&& error) SKR_NOEXCEPT
    : _error(std::move(error)),
      _hasValue(false),
      _unhandled(true)
{
}
template <typename E, typename T>
inline Expected<E, T>::Expected(Expected&& other) SKR_NOEXCEPT
    : _hasValue(other._hasValue),
      _unhandled(other._unhandled)
{
    other._unhandled = false;
    if (_hasValue)
        new (&_value) ValueType(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));
}

template <typename E, typename T>
inline Expected<E, T>& Expected<E, T>::operator=(const ValueType& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    _hasValue  = true;
    _unhandled = false;
    _value     = value;
    return *this;
}

template <typename E, typename T>
inline Expected<E, T>& Expected<E, T>::operator=(ValueType&& value) SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    _hasValue  = true;
    _unhandled = false;
    _value     = std::move(value);
    return *this;
}

template <typename E, typename T>
inline Expected<E, T>& Expected<E, T>::operator=(const E& error) SKR_NOEXCEPT
{
    _hasValue  = false;
    _unhandled = true;
    _error     = error;
    return *this;
}

template <typename E, typename T>
inline Expected<E, T>& Expected<E, T>::operator=(E&& error) SKR_NOEXCEPT
{
    _hasValue  = false;
    _unhandled = true;
    _error     = std::move(error);
    return *this;
}

template <typename E, typename T>
inline Expected<E, T>& Expected<E, T>::operator=(Expected&& other) SKR_NOEXCEPT
{
    if (_hasValue)
        _value.~ValueType();
    else
        _error.~E();

    other._unhandled = false;
    _hasValue        = other._hasValue;
    if (_hasValue)
        new (&_value) ValueType(std::move(other._value));
    else
        new (&_error) E(std::move(other._error));

    return *this;
}

template <typename E, typename T>
inline Expected<E, T>::~Expected() SKR_NOEXCEPT
{
    SKR_ASSERT((_hasValue || !_unhandled) && "contains an error but not handled!");

    if (_hasValue)
        _value.~ValueType();
    else
        _error.~E();
}

template <typename E, typename T>
inline const typename Expected<E, T>::ValueType& Expected<E, T>::value() const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    SKR_ASSERT(_hasValue && "take value in error state!");
    return _value;
}

template <typename E, typename T>
inline typename Expected<E, T>::ValueType& Expected<E, T>::value() SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    SKR_ASSERT(_hasValue && "take value in error state!");
    return _value;
}

template <typename E, typename T>
inline void Expected<E, T>::value() const SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
{
    SKR_ASSERT(_hasValue && "take value in error state!");
}

template <typename E, typename T>
inline void Expected<E, T>::value() SKR_NOEXCEPT requires(ExpectedValue<T>::is_void)
{
    SKR_ASSERT(_hasValue && "take value in error state!");
}

template <typename E, typename T>
inline const E& Expected<E, T>::error() const SKR_NOEXCEPT
{
    SKR_ASSERT(!_hasValue && "take error in value state!");
    return _error;
}

template <typename E, typename T>
inline void Expected<E, T>::mark_handled()
{
    _unhandled = false;
}

template <typename E, typename T>
template <typename V>
inline bool Expected<E, T>::operator==(const V& v) const SKR_NOEXCEPT requires(!ExpectedValue<T>::is_void)
{
    return has_value() && (value() == v);
}
template <typename E, typename T>
inline bool Expected<E, T>::operator==(const E& e) const SKR_NOEXCEPT
{
    return has_error() && (error() == e);
}
template <typename E, typename T>
inline bool Expected<E, T>::operator==(const Expected& other) const SKR_NOEXCEPT
{
    if (has_value() && other.has_value())
        return _value == other._value;
    else if (has_error() && other.has_error())
        return error() == other.error();
    return false;
}

template <typename E, typename T>
template <typename F>
inline Expected<E, T>& Expected<E, T>::error_then(F&& f) SKR_NOEXCEPT
{
    if (!_hasValue && _unhandled)
    {
        f(error());
        _unhandled = false;
    }
    return *this;
}
template <typename E, typename T>
template <typename F>
inline Expected<E, T>& Expected<E, T>::and_then(F&& f) SKR_NOEXCEPT
{
    if (_hasValue)
    {
        if constexpr (ExpectedValue<T>::is_void)
            f();
        else
            f(value());
    }
    return *this;
}

template <typename E, typename T>
Optional<E> Expected<E, T>::take_error()
{
    if (_hasValue)
    {
        return {};
    }
    else
    {
        _unhandled = false;
        return std::move(_error);
    }
}

} // namespace skr

#define SKR_EXPECTED_ENSURE(__EXPR) \
    if (auto zz_expected = (__EXPR); !zz_expected.has_value()) return zz_expected;

#define SKR_EXPECTED_CHECK(__EXPR, __RET) \
    if (!(__EXPR).has_value()) return __RET;

#pragma once
#include <SkrBase/config.h>
#include <type_traits>

namespace skr
{
struct Trigger {
    // ctor & dtor
    Trigger();
    ~Trigger();

    // copy & move
    Trigger(const Trigger& rhs);
    Trigger(Trigger&& rhs);

    // assign & move assign
    Trigger& operator=(const Trigger& rhs);
    Trigger& operator=(Trigger&& rhs);

    // trigger
    void trigger();
    void trigger_if(bool condition);
    void trigger_if(Trigger other);
    void clean();
    bool comsume() const; // const for allow consume, but prevent trigger

    // getter
    bool is_triggered() const;

private:
    mutable bool _triggered = false;
};

template <typename T>
struct Dirty {
    static_assert(!std::is_same_v<T, void>, "if you want a void dirty, use trigger instead");

    // ctor & dtor
    Dirty();
    Dirty(const T& value, bool dirty = false);
    Dirty(T&& value, bool dirty = false);
    ~Dirty();

    // copy & move
    Dirty(const Dirty& rhs);
    Dirty(Dirty&& rhs);

    // assign & move assign
    Dirty& operator=(const Dirty& rhs);
    Dirty& operator=(Dirty&& rhs);
    Dirty& operator=(const T& rhs); // [AUTO-DIRTY]
    Dirty& operator=(T&& rhs);      // [AUTO-DIRTY]

    // dirty
    void dirty();
    void dirty_if(bool condition);
    void clean();
    bool comsume() const; // const for allow consume, but prevent trigger

    // getter
    const T& ref() const;
    T&       ref_w(); // [AUTO-DIRTY]
    const T* ptr() const;
    T*       ptr_w(); // [AUTO-DIRTY]
    bool     is_dirty() const;

    // pointer behavior
    const T* operator->() const;
    // T*       operator->(); // for safe dirty, no implicit reference getter
    const T& operator*() const;
    // T&       operator*(); // for safe dirty, no implicit reference getter

private:
    T            _value = {};
    mutable bool _dirty = false;
};
} // namespace skr

// trigger
namespace skr
{
// ctor & dtor
inline Trigger::Trigger()
{
}
inline Trigger::~Trigger()
{
}

// copy & move
inline Trigger::Trigger(const Trigger& rhs)
    : _triggered()
{
}
inline Trigger::Trigger(Trigger&& rhs)
    : _triggered(rhs._triggered)
{
    rhs._triggered = false;
}

// assign & move assign
inline Trigger& Trigger::operator=(const Trigger& rhs)
{
    _triggered = rhs._triggered;
    return *this;
}
inline Trigger& Trigger::operator=(Trigger&& rhs)
{
    _triggered     = rhs._triggered;
    rhs._triggered = false;
    return *this;
}

// trigger
inline void Trigger::trigger()
{
    _triggered = true;
}
inline void Trigger::trigger_if(bool condition)
{
    if (condition)
    {
        _triggered = true;
    }
}
inline void Trigger::trigger_if(Trigger other)
{
    if (other._triggered)
    {
        _triggered = true;
    }
}
inline void Trigger::clean()
{
    _triggered = false;
}
inline bool Trigger::comsume() const
{
    bool old   = _triggered;
    _triggered = false;
    return old;
}

// getter
inline bool Trigger::is_triggered() const
{
    return _triggered;
}
} // namespace skr

// dirty
namespace skr
{
// ctor & dtor
template <typename T>
inline Dirty<T>::Dirty() = default;
template <typename T>
inline Dirty<T>::Dirty(const T& value, bool dirty)
    : _value(value)
    , _dirty(dirty)
{
}
template <typename T>
inline Dirty<T>::Dirty(T&& value, bool dirty)
    : _value(std::move(value))
    , _dirty(dirty)
{
}
template <typename T>
inline Dirty<T>::~Dirty() = default;

// copy & move
template <typename T>
inline Dirty<T>::Dirty(const Dirty& rhs)
    : _value(rhs._value)
    , _dirty(rhs._dirty)
{
}
template <typename T>
inline Dirty<T>::Dirty(Dirty&& rhs)
    : _value(std::move(rhs._value))
    , _dirty(rhs._dirty)
{
    rhs._dirty = false;
}

// assign & move assign
template <typename T>
inline Dirty<T>& Dirty<T>::operator=(const Dirty& rhs)
{
    _value = rhs._value;
    _dirty = rhs._dirty;
    return *this;
}
template <typename T>
inline Dirty<T>& Dirty<T>::operator=(Dirty&& rhs)
{
    _value     = std::move(rhs._value);
    _dirty     = rhs._dirty;
    rhs._dirty = false;
    return *this;
}
template <typename T>
inline Dirty<T>& Dirty<T>::operator=(const T& rhs)
{
    _value = rhs;
    _dirty = true;
    return *this;
}
template <typename T>
inline Dirty<T>& Dirty<T>::operator=(T&& rhs)
{
    _value = std::move(rhs);
    _dirty = true;
    return *this;
}

// dirty
template <typename T>
inline void Dirty<T>::dirty()
{
    _dirty = true;
}
template <typename T>
inline void Dirty<T>::dirty_if(bool condition)
{
    if (condition)
    {
        _dirty = true;
    }
}
template <typename T>
inline void Dirty<T>::clean()
{
    _dirty = false;
}
template <typename T>
inline bool Dirty<T>::comsume() const
{
    bool old = _dirty;
    _dirty   = false;
    return old;
}

// getter
template <typename T>
inline const T& Dirty<T>::ref() const
{
    return _value;
}
template <typename T>
inline T& Dirty<T>::ref_w()
{
    _dirty = true;
    return _value;
}
template <typename T>
inline const T* Dirty<T>::ptr() const
{
    return &_value;
}
template <typename T>
inline T* Dirty<T>::ptr_w()
{
    _dirty = true;
    return &_value;
}
template <typename T>
inline bool Dirty<T>::is_dirty() const
{
    return _dirty;
}

// pointer behavior
template <typename T>
inline const T* Dirty<T>::operator->() const
{
    return &_value;
}
template <typename T>
inline const T& Dirty<T>::operator*() const
{
    return _value;
}

} // namespace skr
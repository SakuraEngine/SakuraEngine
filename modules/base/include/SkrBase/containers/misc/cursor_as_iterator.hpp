#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
template <typename T>
struct CursorAsIteratorForward : protected T {
    // ctor & copy & move & assign & move assign
    inline CursorAsIteratorForward(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorAsIteratorForward(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorAsIteratorForward(const CursorAsIteratorForward& rhs)            = default;
    inline CursorAsIteratorForward(CursorAsIteratorForward&& rhs)                 = default;
    inline CursorAsIteratorForward& operator=(const CursorAsIteratorForward& rhs) = default;
    inline CursorAsIteratorForward& operator=(CursorAsIteratorForward&& rhs)      = default;

    // getter
    inline decltype(auto) ref() const { return T::ref(); }
    inline decltype(auto) ptr() const
    requires requires(const T& rng) { rng.ptr(); }
    {
        return T::ptr();
    }
    inline decltype(auto) index() const
    requires requires(const T& rng) { rng.index(); }
    {
        return T::index();
    }

    // move & validator
    inline void reset() { T::reset_to_begin(); }
    inline void move_next() { T::move_next(); }
    inline bool has_next() const { return !T::reach_end(); }

    // cursor
    inline const T& as_cursor() const { return *this; }
    inline T&       as_cursor() { return *this; }
};
template <typename T>
struct CursorAsIteratorBackward : protected T {
    // ctor & copy & move & assign & move assign
    inline CursorAsIteratorBackward(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorAsIteratorBackward(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorAsIteratorBackward(const CursorAsIteratorBackward& rhs)            = default;
    inline CursorAsIteratorBackward(CursorAsIteratorBackward&& rhs)                 = default;
    inline CursorAsIteratorBackward& operator=(const CursorAsIteratorBackward& rhs) = default;
    inline CursorAsIteratorBackward& operator=(CursorAsIteratorBackward&& rhs)      = default;

    // getter
    inline decltype(auto) ref() const { return T::ref(); }
    inline decltype(auto) ptr() const
    requires requires(const T& rng) { rng.ptr(); }
    {
        return T::ptr();
    }
    inline decltype(auto) index() const
    requires requires(const T& rng) { rng.index(); }
    {
        return T::index();
    }

    // move & validator
    inline void reset() { T::reset_to_end(); }
    inline void move_next() { T::move_prev(); }
    inline bool has_next() const { return !T::reach_begin(); }

    // cursor
    inline const T& as_cursor() const { return *this; }
    inline T&       as_cursor() { return *this; }
};

template <typename T>
struct CursorAsStlIteratorForward : protected T {
    // ctor & copy & move & assign & move assign
    inline CursorAsStlIteratorForward(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorAsStlIteratorForward(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorAsStlIteratorForward(const CursorAsStlIteratorForward& rhs)            = default;
    inline CursorAsStlIteratorForward(CursorAsStlIteratorForward&& rhs)                 = default;
    inline CursorAsStlIteratorForward& operator=(const CursorAsStlIteratorForward& rhs) = default;
    inline CursorAsStlIteratorForward& operator=(CursorAsStlIteratorForward&& rhs)      = default;

    // compare
    inline friend bool operator==(const CursorAsStlIteratorForward& lhs, const CursorAsStlIteratorForward& rhs)
    {
        return static_cast<const T&>(lhs) == static_cast<const T&>(rhs);
    }
    inline friend bool operator!=(const CursorAsStlIteratorForward& lhs, const CursorAsStlIteratorForward& rhs)
    {
        return !(lhs == rhs);
    }

    // move
    inline CursorAsStlIteratorForward& operator++()
    {
        static_cast<T&>(*this).move_next();
        return *this;
    }
    inline CursorAsStlIteratorForward operator++(int)
    {
        auto tmp = *this;
        static_cast<T&>(*this).move_next();
        return tmp;
    }

    // dereference
    inline decltype(auto) operator*() const { return static_cast<const T&>(*this).ref(); }
    inline decltype(auto) operator*() { return static_cast<T&>(*this).ref(); }
};
template <typename T>
struct CursorAsStlIteratorBackward : protected T {
    // ctor & copy & move & assign & move assign
    inline CursorAsStlIteratorBackward(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorAsStlIteratorBackward(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorAsStlIteratorBackward(const CursorAsStlIteratorBackward& rhs)            = default;
    inline CursorAsStlIteratorBackward(CursorAsStlIteratorBackward&& rhs)                 = default;
    inline CursorAsStlIteratorBackward& operator=(const CursorAsStlIteratorBackward& rhs) = default;
    inline CursorAsStlIteratorBackward& operator=(CursorAsStlIteratorBackward&& rhs)      = default;

    // compare
    inline friend bool operator==(const CursorAsStlIteratorBackward& lhs, const CursorAsStlIteratorBackward& rhs)
    {
        return static_cast<const T&>(lhs) == static_cast<const T&>(rhs);
    }
    inline friend bool operator!=(const CursorAsStlIteratorBackward& lhs, const CursorAsStlIteratorBackward& rhs)
    {
        return !(lhs == rhs);
    }

    // move
    inline CursorAsStlIteratorBackward& operator++()
    {
        static_cast<T&>(*this).move_prev();
        return *this;
    }
    inline CursorAsStlIteratorBackward operator++(int)
    {
        auto tmp = *this;
        static_cast<T&>(*this).move_prev();
        return tmp;
    }

    // dereference
    inline decltype(auto) operator*() const { return static_cast<const T&>(*this).ref(); }
    inline decltype(auto) operator*() { return static_cast<T&>(*this).ref(); }
};
} // namespace skr::container
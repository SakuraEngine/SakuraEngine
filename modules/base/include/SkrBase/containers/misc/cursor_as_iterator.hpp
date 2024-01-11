#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/iterator_as_range.hpp"

namespace skr::container
{
template <typename T>
struct CursorIter : protected T {
    using DataType = typename T::DataType;

    // ctor & copy & move & assign & move assign
    inline CursorIter(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorIter(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorIter(const CursorIter& rhs)            = default;
    inline CursorIter(CursorIter&& rhs)                 = default;
    inline CursorIter& operator=(const CursorIter& rhs) = default;
    inline CursorIter& operator=(CursorIter&& rhs)      = default;

    // getter
    inline DataType& ref() const { return T::ref(); }
    inline DataType* ptr() const
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

    // cast
    inline const T& cursor() const { return *this; }
    inline T&       cursor() { return *this; }
    inline auto     as_range() const { return IteratorAsRange<CursorIter>(*this); }
    inline auto     as_range() { return IteratorAsRange<CursorIter>(*this); }
};
template <typename T>
struct CursorIterInv : protected T {
    using DataType = typename T::DataType;

    // ctor & copy & move & assign & move assign
    inline CursorIterInv(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline CursorIterInv(const T& rhs)
        : T(rhs)
    {
    }
    inline CursorIterInv(const CursorIterInv& rhs)            = default;
    inline CursorIterInv(CursorIterInv&& rhs)                 = default;
    inline CursorIterInv& operator=(const CursorIterInv& rhs) = default;
    inline CursorIterInv& operator=(CursorIterInv&& rhs)      = default;

    // getter
    inline DataType& ref() const { return T::ref(); }
    inline DataType* ptr() const
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

    // cast
    inline const T& cursor() const { return *this; }
    inline T&       cursor() { return *this; }
    inline auto     as_range() const { return IteratorAsRange<CursorIterInv>(*this); }
    inline auto     as_range() { return IteratorAsRange<CursorIterInv>(*this); }
};

template <typename T>
struct StlStyleCursorIter : protected T {
    using DataType = typename T::DataType;

    // ctor & copy & move & assign & move assign
    inline StlStyleCursorIter(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline StlStyleCursorIter(const T& rhs)
        : T(rhs)
    {
    }
    inline StlStyleCursorIter(const StlStyleCursorIter& rhs)            = default;
    inline StlStyleCursorIter(StlStyleCursorIter&& rhs)                 = default;
    inline StlStyleCursorIter& operator=(const StlStyleCursorIter& rhs) = default;
    inline StlStyleCursorIter& operator=(StlStyleCursorIter&& rhs)      = default;

    // compare
    inline friend bool operator==(const StlStyleCursorIter& lhs, const StlStyleCursorIter& rhs)
    {
        return static_cast<const T&>(lhs) == static_cast<const T&>(rhs);
    }
    inline friend bool operator!=(const StlStyleCursorIter& lhs, const StlStyleCursorIter& rhs)
    {
        return !(lhs == rhs);
    }

    // move
    inline StlStyleCursorIter& operator++()
    {
        T::move_next();
        return *this;
    }
    inline StlStyleCursorIter operator++(int)
    {
        auto tmp = *this;
        T::move_next();
        return tmp;
    }

    // dereference
    inline const DataType& operator*() const { return T::ref(); }
    inline DataType&       operator*() { return T::ref(); }
};
template <typename T>
struct StlStyleCursorIterInv : protected T {
    using DataType = typename T::DataType;

    // ctor & copy & move & assign & move assign
    inline StlStyleCursorIterInv(T&& rhs)
        : T(std::move(rhs))
    {
    }
    inline StlStyleCursorIterInv(const T& rhs)
        : T(rhs)
    {
    }
    inline StlStyleCursorIterInv(const StlStyleCursorIterInv& rhs)            = default;
    inline StlStyleCursorIterInv(StlStyleCursorIterInv&& rhs)                 = default;
    inline StlStyleCursorIterInv& operator=(const StlStyleCursorIterInv& rhs) = default;
    inline StlStyleCursorIterInv& operator=(StlStyleCursorIterInv&& rhs)      = default;

    // compare
    inline friend bool operator==(const StlStyleCursorIterInv& lhs, const StlStyleCursorIterInv& rhs)
    {
        return static_cast<const T&>(lhs) == static_cast<const T&>(rhs);
    }
    inline friend bool operator!=(const StlStyleCursorIterInv& lhs, const StlStyleCursorIterInv& rhs)
    {
        return !(lhs == rhs);
    }

    // move
    inline StlStyleCursorIterInv& operator++()
    {
        T::move_prev();
        return *this;
    }
    inline StlStyleCursorIterInv operator++(int)
    {
        auto tmp = *this;
        T::move_prev();
        return tmp;
    }

    // dereference
    inline const DataType& operator*() const { return T::ref(); }
    inline DataType&       operator*() { return T::ref(); }
};
} // namespace skr::container
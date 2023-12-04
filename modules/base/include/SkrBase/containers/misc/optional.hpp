#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/memory/memory_ops.hpp"

namespace skr::container
{
template <typename T>
struct Optional {
    // ctor & dtor
    Optional();
    Optional(const T& value);
    Optional(T&& value);
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
    template <typename U>
    Optional& operator=(const Optional<U>& other);
    template <typename U>
    Optional& operator=(Optional<U>&& other);

    // value checker
    operator bool() const;
    bool has_value() const;

    // getter
    T&        operator*();
    const T&  operator*() const;
    T&        value() &;
    const T&  value() const&;
    T&&       value() &&;
    const T&& value() const&&;

    // modifier
    void reset();
    template <typename... Args>
    void emplace(Args&&... args);

private:
    Placeholder<T> _placeholder = {};
    bool           _has_value   = false;
};
} // namespace skr::container
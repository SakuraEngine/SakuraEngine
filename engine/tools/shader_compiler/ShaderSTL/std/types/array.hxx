#pragma once
#include "../attributes.hxx"
#include "../type_traits.hxx"

template <typename T, uint32 Size>
struct [[builtin("array")]] Array
{
    static constexpr uint32 N = Size;

    using value_type = T;
    using reference = T&;
    using const_reference = T;
    using size_type = uint32;
    using difference_type = int32;

    template <typename... Args>
    constexpr Array(Args... args)
    {
        set<0>(args...);
    }
    constexpr Array() = default;

    template <size_type start, typename... Args>
    constexpr void set(T v, Args... args)
    {
        set(start, v);
        set<start + 1>(args...);
    }
    template <size_type start>
    constexpr void set(T v) { set(start, v); }
    constexpr void set(size_type loc, T v) { operator[](loc) = v; }

    constexpr void fill(const T& u)
    {
        for (size_type i = 0; i < N; ++i)
        {
            operator[](i) = u;
        }
    }

    // capacity
    [[nodiscard]] constexpr bool empty() noexcept
    {
        return false;
    }
    [[nodiscard]] constexpr size_type size() noexcept
    {
        return N;
    }
    [[nodiscard]] constexpr size_type max_size() noexcept
    {
        return N;
    }

    // element access
    [[nodiscard]] [[access]] constexpr reference operator[](size_type n)
    {
        return v[n];
    }
    [[nodiscard]] [[access]] constexpr const_reference operator[](size_type n) const
    {
        return v[n];
    }
    [[nodiscard]] [[access]] constexpr reference at(size_type n)
    {
        return v[n];
    }
    [[nodiscard]] [[access]] constexpr const_reference at(size_type n) const
    {
        return v[n];
    }
    [[nodiscard]] [[access]] constexpr reference front()
    {
        return v[0];
    }
    [[nodiscard]] [[access]] constexpr const_reference front() const
    {
        return v[0];
    }
    [[nodiscard]] [[access]] constexpr reference back()
    {
        return v[N - 1];
    }
    [[nodiscard]] [[access]] constexpr const_reference back() const
    {
        return v[N - 1];
    }

private:
    // DONT EDIT THIS FIELD LAYOUT
    T v[N];
};

template <class T, class... U>
Array(T, U...) -> Array<T, 1 + sizeof...(U)>;
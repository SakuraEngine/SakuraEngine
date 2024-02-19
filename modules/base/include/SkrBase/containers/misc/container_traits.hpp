#pragma once
#include <array>
#include <initializer_list>
#include <type_traits>

namespace skr::container
{
template <typename T>
struct ContainerTraits {
    constexpr static bool is_linear_memory = false; // data(), size()
    constexpr static bool has_size         = false; // size()
    constexpr static bool is_iterable      = false; // begin(), end()
};

template <typename T>
struct ContainerTraits<std::initializer_list<T>> {
    constexpr static bool is_linear_memory = true; // data(), size()
    constexpr static bool has_size         = true; // size()
    constexpr static bool is_iterable      = true; // begin(), end()

    static inline const T* data(const std::initializer_list<T>& init) { return init.begin(); }
    static inline size_t   size(const std::initializer_list<T>& init) { return init.size(); }

    static inline const T* begin(const std::initializer_list<T>& init) { return init.begin(); }
    static inline const T* end(const std::initializer_list<T>& init) { return init.end(); }
};

template <typename T, size_t kSize>
struct ContainerTraits<std::array<T, kSize>> {
    constexpr static bool is_linear_memory = true; // data(), size()
    constexpr static bool has_size         = true; // size()
    constexpr static bool is_iterable      = true; // begin(), end()

    static inline const T* data(const std::array<T, kSize>& arr) { return arr.data(); }
    static inline T*       data(std::array<T, kSize>& arr) { return arr.data(); }
    static inline size_t   size(const std::array<T, kSize>& arr) { return arr.size(); }

    static inline auto begin(const std::array<T, kSize>& arr) { return arr.begin(); }
    static inline auto end(const std::array<T, kSize>& arr) { return arr.end(); }
    static inline auto begin(std::array<T, kSize>& arr) { return arr.begin(); }
    static inline auto end(std::array<T, kSize>& arr) { return arr.end(); }
};

template <typename T>
concept EachAbleContainer = ContainerTraits<std::decay_t<T>>::is_linear_memory || ContainerTraits<std::decay_t<T>>::is_iterable;
template <typename T>
concept LinearMemoryContainer = ContainerTraits<std::decay_t<T>>::is_linear_memory;
} // namespace skr::container
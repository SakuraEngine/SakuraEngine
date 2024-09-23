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
    constexpr static bool is_reservable    = false; // reserve()

    using ElementType = void;
};

template <typename T>
struct ContainerTraits<std::initializer_list<T>> {
    constexpr static bool is_linear_memory = true;  // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = false; // reserve()

    using ElementType = T;
    using SizeType    = size_t;

    inline static const T* data(const std::initializer_list<T>& init) { return init.begin(); }
    inline static size_t   size(const std::initializer_list<T>& init) { return init.size(); }

    inline static const T* begin(const std::initializer_list<T>& init) { return init.begin(); }
    inline static const T* end(const std::initializer_list<T>& init) { return init.end(); }
};

template <typename T, size_t kSize>
struct ContainerTraits<std::array<T, kSize>> {
    constexpr static bool is_linear_memory = true;  // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = false; // reserve()

    using ElementType = T;
    using SizeType    = size_t;

    inline static const T* data(const std::array<T, kSize>& arr) { return arr.data(); }
    inline static T*       data(std::array<T, kSize>& arr) { return arr.data(); }
    inline static size_t   size(const std::array<T, kSize>& arr) { return arr.size(); }

    inline static auto begin(const std::array<T, kSize>& arr) { return arr.begin(); }
    inline static auto end(const std::array<T, kSize>& arr) { return arr.end(); }
    inline static auto begin(std::array<T, kSize>& arr) { return arr.begin(); }
    inline static auto end(std::array<T, kSize>& arr) { return arr.end(); }
};

// TODO. add ElementType check for better error message
template <typename T>
concept EachAbleContainer = ContainerTraits<std::decay_t<T>>::is_linear_memory || ContainerTraits<std::decay_t<T>>::is_iterable;
template <typename T>
concept LinearMemoryContainer = ContainerTraits<std::decay_t<T>>::is_linear_memory;

// TODO. container append adaptor

template <typename T, typename Elem>
concept HasAppend = requires(T t, Elem e) {
    t.append(e);
};
template <typename T, typename Elem>
concept HasAdd = requires(T t, Elem e) {
    t.add(e);
};

} // namespace skr::container

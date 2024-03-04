#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"

namespace skr::container
{
inline static constexpr size_t kDynamicExtent = npos_of<size_t>;

namespace __helper
{
template <size_t Extent, size_t Offset, size_t Count>
inline static constexpr size_t subspan_extent = (Count != kDynamicExtent ? Count : (Extent != kDynamicExtent ? (Extent - Offset) : kDynamicExtent));
} // namespace __helper

template <typename T, typename TSize, size_t Extent = kDynamicExtent>
struct Span {
    using DataType = T;

    // ctor & dtor
    constexpr Span();
    constexpr Span(T* data, TSize size);
    constexpr Span(T* begin, T* end);
    template <size_t N, typename = std::enable_if_t<(Extent == kDynamicExtent || N == Extent)>>
    constexpr Span(T (&arr)[N]);
    template <LinearMemoryContainer U>
    constexpr Span(U&& container);
    ~Span();

    // copy & move
    constexpr Span(const Span& other);
    constexpr Span(Span&& other);

    // assign & move assign
    constexpr Span& operator=(const Span& other);
    constexpr Span& operator=(Span&& other);

    // subviews
    template <size_t Count>
    constexpr Span<T, TSize, Count>          first() const;
    constexpr Span<T, TSize, kDynamicExtent> first(TSize count) const;
    template <size_t Count>
    constexpr Span<T, TSize, Count>          last() const;
    constexpr Span<T, TSize, kDynamicExtent> last(TSize count) const;
    template <size_t Offset, size_t Count = kDynamicExtent>
    constexpr Span<T, TSize, __helper::subspan_extent<Extent, Offset, Count>> subspan() const;
    constexpr Span<T, TSize, kDynamicExtent>                                  subspan(size_t offset, size_t count = kDynamicExtent) const;

    // getter
    constexpr T*    data() const;
    constexpr TSize size() const;
    constexpr bool  empty() const;

    // element access
    constexpr T& operator[](TSize idx) const;

    // iterator
    constexpr T* begin() const;
    constexpr T* end() const;

    // validator
    constexpr bool is_valid_index(TSize idx) const;
    constexpr bool is_valid_ptr(T* ptr) const;

private:
    T*     _data = nullptr;
    size_t _size = 0;
};
} // namespace skr::container

namespace skr::container
{
// ctor & dtor
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span()
{
    static_assert(Extent == kDynamicExtent || Extent == 0, "impossible to default construct a span with a fixed Extent different than 0");
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span(T* data, TSize size)
    : _data(data)
    , _size(size)
{
    static_assert(Extent == kDynamicExtent || Extent == 0, "impossible to default construct a span with a fixed Extent different than 0");
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span(T* begin, T* end)
    : _data(begin)
    , _size(static_cast<TSize>(end - begin))
{
    static_assert(Extent == kDynamicExtent || Extent == 0, "impossible to default construct a span with a fixed Extent different than 0");
}
template <typename T, typename TSize, size_t Extent>
template <size_t N, typename>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span(T (&arr)[N])
    : _data(arr)
    , _size(static_cast<TSize>(N))
{
    static_assert(Extent == kDynamicExtent || Extent == 0 || Extent <= N, "impossible to default construct a span with a fixed Extent different than 0");
}
template <typename T, typename TSize, size_t Extent>
template <LinearMemoryContainer U>
constexpr Span<T, TSize, Extent>::Span(U&& container)
    : _data(ContainerTraits<std::decay_t<U>>::data(std::forward<U>(container)))
    , _size(ContainerTraits<std::decay_t<U>>::size(std::forward<U>(container)))
{
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE Span<T, TSize, Extent>::~Span() = default;

// copy & move
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span(const Span& other) = default;
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>::Span(Span&& other) = default;

// assign & move assign
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>& Span<T, TSize, Extent>::operator=(const Span& other) = default;
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, Extent>& Span<T, TSize, Extent>::operator=(Span&& other) = default;

// subviews
template <typename T, typename TSize, size_t Extent>
template <size_t Count>
SKR_INLINE constexpr Span<T, TSize, Count> Span<T, TSize, Extent>::first() const
{
    SKR_ASSERT(Count <= size() && "undefined behavior accessing out of bounds");
    return { data(), static_cast<TSize>(Count) };
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, kDynamicExtent> Span<T, TSize, Extent>::first(TSize count) const
{
    SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
    return { data(), static_cast<TSize>(count) };
}
template <typename T, typename TSize, size_t Extent>
template <size_t Count>
SKR_INLINE constexpr Span<T, TSize, Count> Span<T, TSize, Extent>::last() const
{
    SKR_ASSERT(Count <= size() && "undefined behavior accessing out of bounds");
    return { data() + size() - Count, static_cast<TSize>(Count) };
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, kDynamicExtent> Span<T, TSize, Extent>::last(TSize count) const
{
    SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
    return { data() + size() - count, static_cast<TSize>(count) };
}
template <typename T, typename TSize, size_t Extent>
template <size_t Offset, size_t Count>
SKR_INLINE constexpr Span<T, TSize, __helper::subspan_extent<Extent, Offset, Count>> Span<T, TSize, Extent>::subspan() const
{
    SKR_ASSERT(Offset <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(Count == kDynamicExtent || Count <= (size() - Offset) && "undefined behaviour exceeding size of span");

    return { data() + Offset, TSize(Count == kDynamicExtent ? size() - Offset : Count) };
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr Span<T, TSize, kDynamicExtent> Span<T, TSize, Extent>::subspan(size_t offset, size_t count) const
{
    SKR_ASSERT(offset <= size() && "undefined behaviour accessing out of bounds");
    SKR_ASSERT(count == kDynamicExtent || count <= (size() - offset) && "undefined behaviour exceeding size of span");

    return { data() + offset, TSize(count == kDynamicExtent ? size() - offset : count) };
}

// getter
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr T* Span<T, TSize, Extent>::data() const
{
    return _data;
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr TSize Span<T, TSize, Extent>::size() const
{
    return _size;
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr bool Span<T, TSize, Extent>::empty() const
{
    return size() == 0;
}

// element access
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr T& Span<T, TSize, Extent>::operator[](TSize idx) const
{
    SKR_ASSERT(!empty() && "undefined behavior accessing an empty span");
    SKR_ASSERT(is_valid_index(idx) && "undefined behavior accessing out of bounds");

    return _data[idx];
}

// iterator
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr T* Span<T, TSize, Extent>::begin() const
{
    return _data;
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr T* Span<T, TSize, Extent>::end() const
{
    return _data + _size;
}

// validator
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr bool Span<T, TSize, Extent>::is_valid_index(TSize idx) const
{
    return idx >= 0 && idx < _size;
}
template <typename T, typename TSize, size_t Extent>
SKR_INLINE constexpr bool Span<T, TSize, Extent>::is_valid_ptr(T* ptr) const
{
    return ptr >= begin() && ptr < end();
}

} // namespace skr::container

namespace skr::container
{
template <typename T, typename TSize, size_t Extent>
struct ContainerTraits<Span<T, TSize, Extent>> {
    constexpr static bool is_linear_memory = true; // data(), size()
    constexpr static bool has_size         = true; // size()
    constexpr static bool is_iterable      = true; // begin(), end()

    static inline const typename Span<T, TSize, Extent>::DataType* data(const Span<T, TSize, Extent>& container) { return container.data(); }
    static inline typename Span<T, TSize, Extent>::DataType*       data(Span<T, TSize, Extent>& container) { return container.data(); }
    static inline size_t                                           size(const Span<T, TSize, Extent>& container) { return container.size(); }

    static inline auto begin(const Span<T, TSize, Extent>& container) noexcept { return container.begin(); }
    static inline auto end(const Span<T, TSize, Extent>& container) noexcept { return container.end(); }
    static inline auto begin(Span<T, TSize, Extent>& container) noexcept { return container.begin(); }
    static inline auto end(Span<T, TSize, Extent>& container) noexcept { return container.end(); }
};
} // namespace skr::container
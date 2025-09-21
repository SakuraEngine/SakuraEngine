#pragma once
#include <string_view>
#include "SkrBase/config.h"
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/template/concepts.hpp"
#include "internal/constexpr-xxh3.hpp"
#include <concepts>
#include <SkrBase/misc/hash.h>

namespace skr
{
template <constexpr_xxh3::BytesType Bytes>
consteval uint64_t consteval_hash(const Bytes& input)
{
    return constexpr_xxh3::XXH3_64bits_const(input);
}
} // namespace skr

namespace skr
{
template <typename T>
struct Hash;

// concepts
namespace concepts
{
template <typename U, typename T = U>
concept HasEmbeddedHasherMember = requires(const T& t, const U& u) {
    { t._skr_hash(u) } -> std::same_as<skr_hash>;
};
template <typename U, typename T = U>
concept HasEmbeddedHasherStatic = requires(const T& t, const U& u) {
    { T::_skr_hash(u) } -> std::same_as<skr_hash>;
};
template <typename U, typename T = U>
concept HasEmbeddedHasher = HasEmbeddedHasherMember<T, U> || HasEmbeddedHasherStatic<T, U>;

template <typename U, typename T = U>
concept HasHasher = requires(const T& t, const U& u) {
    { ::skr::Hash<T>{}(u) } -> std::same_as<skr_hash>;
};

} // namespace concepts

template <typename T>
struct Hash
{
};

template <concepts::HasEmbeddedHasher T>
struct Hash<T>
{
    inline skr_hash operator()(const T& p) const
    {
        if constexpr (concepts::HasEmbeddedHasherMember<T>)
        {
            return p._skr_hash(p);
        }
        else if constexpr (concepts::HasEmbeddedHasherStatic<T>)
        {
            return T::_skr_hash(p);
        }
    }
    template <concepts::HasEmbeddedHasher<T> U>
    inline skr_hash operator()(const U& u) const
    {
        if constexpr (concepts::HasEmbeddedHasherMember<U, T>)
        {
            return u._skr_hash(u);
        }
        else if constexpr (concepts::HasEmbeddedHasherStatic<U, T>)
        {
            return T::_skr_hash(u);
        }
    }
};

template <concepts::Enum T>
struct Hash<T>
{
    inline skr_hash operator()(const T& p) const
    {
        return static_cast<skr_hash>(p);
    }
};

// hash combine
SKR_INLINE constexpr skr_hash hash_combine(skr_hash seed, skr_hash combine)
{
    return seed ^ (combine + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

} // namespace skr

// primitive type hash
namespace skr
{
#define SKR_IMPL_HASH_CAST(type)                                 \
    template <>                                                  \
    struct Hash<type>                                            \
    {                                                            \
        SKR_INLINE constexpr Hash() {}                           \
        SKR_INLINE constexpr skr_hash operator()(type val) const \
        {                                                        \
            return static_cast<skr_hash>(val);                   \
        }                                                        \
    };

// helper
SKR_INLINE skr_hash fold_if_needed(uint64_t v)
{
    if constexpr (sizeof(skr_hash) == 4)
    {
        return static_cast<skr_hash>(v ^ (v >> 32));
    }
    else if constexpr (sizeof(skr_hash) == 8)
    {
        return static_cast<skr_hash>(v);
    }
}

// impl for pointer
template <typename T>
struct Hash<T*>
{
    SKR_INLINE skr_hash operator()(const T* val) const
    {
        return static_cast<skr_hash>(reinterpret_cast<const uintptr_t>(val));
    }
};

// impl for boolean
SKR_IMPL_HASH_CAST(bool)

// impl for char
SKR_IMPL_HASH_CAST(char)
SKR_IMPL_HASH_CAST(signed char)
SKR_IMPL_HASH_CAST(unsigned char)
SKR_IMPL_HASH_CAST(wchar_t)
SKR_IMPL_HASH_CAST(char8_t)
SKR_IMPL_HASH_CAST(char16_t)
SKR_IMPL_HASH_CAST(char32_t)

// impl for int
SKR_IMPL_HASH_CAST(int16_t)
SKR_IMPL_HASH_CAST(uint16_t)
SKR_IMPL_HASH_CAST(int32_t)
SKR_IMPL_HASH_CAST(uint32_t)
SKR_IMPL_HASH_CAST(int64_t)
SKR_IMPL_HASH_CAST(uint64_t)

// impl for float
template <>
struct Hash<float>
{
    SKR_INLINE skr_hash operator()(float val) const noexcept
    {
        // -0.0 and 0.0 should return same hash
        uint32_t* as_int = reinterpret_cast<uint32_t*>(&val);
        return (val == 0) ? static_cast<skr_hash>(0) :
                            static_cast<skr_hash>(*as_int);
    }
};

// impl for double
template <>
struct Hash<double>
{
    SKR_INLINE skr_hash operator()(double val) const noexcept
    {
        // -0.0 and 0.0 should return same hash
        uint64_t* as_int = reinterpret_cast<uint64_t*>(&val);
        return (val == 0) ? static_cast<skr_hash>(0) :
                            fold_if_needed(*as_int);
    }
};

#undef SKR_IMPL_HASH_CAST
} // namespace skr

// impl for string
namespace skr
{
// raw char
template <>
struct Hash<char*>
{
    SKR_INLINE skr_hash operator()(const char* p) const
    {
        uint32_t c, result = 2166136261U; // FNV1 hash. Perhaps the best string hash. Intentionally uint32_t instead of skr_hash, so the behavior is the same regardless of size.
        while ((c = (uint8_t)*p++) != 0)  // Using '!=' disables compiler warnings.
            result = (result * 16777619) ^ c;
        return (skr_hash)result;
    }
};
template <>
struct Hash<const char*>
{
    SKR_INLINE skr_hash operator()(const char* p) const
    {
        return Hash<char*>()(p);
    }
};

// wchar_t
template <>
struct Hash<wchar_t*>
{
    SKR_INLINE skr_hash operator()(const wchar_t* p) const
    {
        uint32_t c, result = 2166136261U; // Intentionally uint32_t instead of skr_hash, so the behavior is the same regardless of size.
        while ((c = (uint16_t)*p++) != 0) // cast to unsigned 16 bit.
            result = (result * 16777619) ^ c;
        return (skr_hash)result;
    }
};
template <>
struct Hash<const wchar_t*>
{
    SKR_INLINE skr_hash operator()(const wchar_t* p) const
    {
        return Hash<wchar_t*>()(p);
    }
};

// char 8
template <>
struct Hash<char8_t*>
{
    SKR_INLINE skr_hash operator()(const char8_t* p) const
    {
        uint32_t c, result = 2166136261U; // Intentionally uint32_t instead of skr_hash, so the behavior is the same regardless of size.
        while ((c = (uint8_t)*p++) != 0)  // cast to unsigned 8 bit.
            result = (result * 16777619) ^ c;
        return (skr_hash)result;
    }
};
template <>
struct Hash<const char8_t*>
{
    SKR_INLINE skr_hash operator()(const char8_t* p) const
    {
        return Hash<char8_t*>()(p);
    }
};

// char 16
template <>
struct Hash<char16_t*>
{
    SKR_INLINE skr_hash operator()(const char16_t* p) const
    {
        uint32_t c, result = 2166136261U; // Intentionally uint32_t instead of skr_hash, so the behavior is the same regardless of size.
        while ((c = (uint16_t)*p++) != 0) // cast to unsigned 16 bit.
            result = (result * 16777619) ^ c;
        return (skr_hash)result;
    }
};
template <>
struct Hash<const char16_t*>
{
    SKR_INLINE skr_hash operator()(const char16_t* p) const
    {
        return Hash<char16_t*>()(p);
    }
};

// char 32
template <>
struct Hash<char32_t*>
{
    SKR_INLINE skr_hash operator()(const char32_t* p) const
    {
        uint32_t c, result = 2166136261U; // Intentionally uint32_t instead of skr_hash, so the behavior is the same regardless of size.
        while ((c = (uint32_t)*p++) != 0) // cast to unsigned 32 bit.
            result = (result * 16777619) ^ c;
        return (skr_hash)result;
    }
};
template <>
struct Hash<const char32_t*>
{
    SKR_INLINE skr_hash operator()(const char32_t* p) const
    {
        return Hash<char32_t*>()(p);
    }
};
} // namespace skr
#pragma once
#include <SkrBase/misc/swap.hpp>
#include <SkrBase/memory/memory_traits.hpp>
#include "SkrBase/types/guid.h"

// generic id for generic system use
namespace skr
{
static constexpr GUID kKVPairGenericId = u8"bcce8d48-15c1-4b52-9c38-5fe7f934d63e"_guid;
}

namespace skr::container
{
template <typename K, typename V>
struct KVPair {
    K key   = {};
    V value = {};

    // constructor
    KVPair();
    KVPair(const K& k, const V& v);
    KVPair(const K& k, V&& v);
    KVPair(K&& k, const V& v);
    KVPair(K&& k, V&& v);
    template <typename UK, typename UV>
    KVPair(UK&& u1, UV&& u2);

    // copy & move
    KVPair(const KVPair& other);
    KVPair(KVPair&& other);
    template <typename UK, typename UV>
    KVPair(const KVPair<UK, UV>& other);
    template <typename UK, typename UV>
    KVPair(KVPair<UK, UV>&& other);

    // assign
    KVPair& operator=(const KVPair& other);
    KVPair& operator=(KVPair&& other);
    template <typename UK, typename UV>
    KVPair& operator=(const KVPair<UK, UV>& other);
    template <typename UK, typename UV>
    KVPair& operator=(KVPair<UK, UV>&& other);

    // compare
    bool operator==(const KVPair& other) const;
    bool operator!=(const KVPair& other) const;
    bool operator<(const KVPair& other) const;
    bool operator>(const KVPair& other) const;
    bool operator<=(const KVPair& other) const;
    bool operator>=(const KVPair& other) const;
};
} // namespace skr::container

// swap & memory traits
namespace skr
{
template <typename K, typename V>
struct Swap<::skr::container::KVPair<K, V>> {
    static void call(::skr::container::KVPair<K, V>& a, ::skr::container::KVPair<K, V>& b)
    {
        Swap<K>::call(a.key, b.key);
        Swap<V>::call(a.value, b.value);
    }
};
} // namespace skr

// memory traits
namespace skr::memory
{
template <typename K, typename V>
struct MemoryTraits<::skr::container::KVPair<K, V>, ::skr::container::KVPair<K, V>> {
    static constexpr bool use_ctor        = MemoryTraits<K>::use_ctor || MemoryTraits<V>::use_ctor;
    static constexpr bool use_dtor        = MemoryTraits<K>::use_dtor || MemoryTraits<V>::use_dtor;
    static constexpr bool use_copy        = MemoryTraits<K>::use_copy || MemoryTraits<V>::use_copy;
    static constexpr bool use_move        = MemoryTraits<K>::use_move || MemoryTraits<V>::use_move;
    static constexpr bool use_assign      = MemoryTraits<K>::use_assign || MemoryTraits<V>::use_assign;
    static constexpr bool use_move_assign = MemoryTraits<K>::use_move_assign || MemoryTraits<V>::use_move_assign;

    static constexpr bool need_dtor_after_move = MemoryTraits<K>::need_dtor_after_move || MemoryTraits<V>::need_dtor_after_move;

    static constexpr bool use_realloc = MemoryTraits<K>::use_realloc && MemoryTraits<V>::use_realloc;

    static constexpr bool use_compare = MemoryTraits<K>::use_compare || MemoryTraits<V>::use_compare;
};
} // namespace skr::memory

namespace skr::container
{
// constructor
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair()
{
    // do nothing
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(const K& k, const V& v)
    : key(k)
    , value(v)
{
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(const K& k, V&& v)
    : key(k)
    , value(std::move(v))
{
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(K&& k, const V& v)
    : key(std::move(k))
    , value(v)
{
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(K&& k, V&& v)
    : key(std::move(k))
    , value(std::move(v))
{
}
template <typename K, typename V>
template <typename UK, typename UV>
SKR_INLINE KVPair<K, V>::KVPair(UK&& uk, UV&& uv)
    : key(std::forward<UK>(uk))
    , value(std::forward<UV>(uv))
{
}

// copy & move
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(const KVPair& other)
    : key(other.key)
    , value(other.value)
{
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>::KVPair(KVPair&& other)
    : key(std::move(other.key))
    , value(std::move(other.value))
{
}
template <typename K, typename V>
template <typename UK, typename UV>
SKR_INLINE KVPair<K, V>::KVPair(const KVPair<UK, UV>& other)
    : key(other.key)
    , value(other.value)
{
}
template <typename K, typename V>
template <typename UK, typename UV>
SKR_INLINE KVPair<K, V>::KVPair(KVPair<UK, UV>&& other)
    : key(std::move(other.key))
    , value(std::move(other.value))
{
}

// assign
template <typename K, typename V>
SKR_INLINE KVPair<K, V>& KVPair<K, V>::operator=(const KVPair& other)
{
    key   = other.key;
    value = other.value;
    return *this;
}
template <typename K, typename V>
SKR_INLINE KVPair<K, V>& KVPair<K, V>::operator=(KVPair&& other)
{
    key   = std::move(other.key);
    value = std::move(other.value);
    return *this;
}
template <typename K, typename V>
template <typename UK, typename UV>
SKR_INLINE KVPair<K, V>& KVPair<K, V>::operator=(const KVPair<UK, UV>& other)
{
    key   = other.key;
    value = other.value;
    return *this;
}
template <typename K, typename V>
template <typename UK, typename UV>
SKR_INLINE KVPair<K, V>& KVPair<K, V>::operator=(KVPair<UK, UV>&& other)
{
    key   = std::move(other.key);
    value = std::move(other.value);
    return *this;
}

// compare
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator==(const KVPair& rhs) const
{
    return key == rhs.key && value == rhs.value;
}
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator!=(const KVPair& rhs) const
{
    return !((*this) == rhs);
}
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator<(const KVPair& rhs) const
{
    return key < rhs.key || (!(rhs.key < key) && value < rhs.value);
}
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator>(const KVPair& rhs) const
{
    return rhs < (*this);
}
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator<=(const KVPair& rhs) const
{
    return !(rhs < (*this));
}
template <typename K, typename V>
SKR_INLINE bool KVPair<K, V>::operator>=(const KVPair& rhs) const
{
    return !((*this) < rhs);
}
} // namespace skr::container
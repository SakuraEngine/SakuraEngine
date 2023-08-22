#pragma once
#include <type_traits>
#include "SkrBase/config.h"
#include "SkrBase/containers/key_traits.hpp"

namespace skr
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

template <typename K, typename V>
struct MapKey {
    SKR_INLINE constexpr K&       operator()(KVPair<K, V>& pair) const { return pair.key; }
    SKR_INLINE constexpr const K& operator()(const KVPair<K, V>& pair) const { return pair.key; }
};

template <typename K, typename V>
struct KeyTraits<KVPair<K, V>> {
    using KeyType       = K;
    using KeyMapperType = MapKey<K, V>;
};
} // namespace skr

// TODO. skr swap
namespace std
{
template <typename K, typename V>
SKR_INLINE void swap(::skr::KVPair<K, V>& a, ::skr::KVPair<K, V>& b)
{
    ::std::swap(a.key, b.key);
    ::std::swap(a.value, b.value);
}
} // namespace std

namespace skr
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
} // namespace skr

// TODO. integrate std
// std::swap
// std::get
// std::tuple_size
// std::tuple_element
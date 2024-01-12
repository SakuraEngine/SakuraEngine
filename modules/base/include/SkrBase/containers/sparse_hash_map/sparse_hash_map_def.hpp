#pragma once
#include "SkrBase/algo/utils.hpp"
#include "SkrBase/config.h"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"

namespace skr::container
{
template <typename K, typename V, typename TS, typename THash, bool kConst>
struct SparseHashMapDataRef : private SparseHashSetDataRef<KVPair<K, V>, TS, THash, kConst> {
    using Super     = SparseHashSetDataRef<KVPair<K, V>, TS, THash, kConst>;
    using PairType  = std::conditional_t<kConst, const KVPair<K, V>, KVPair<K, V>>;
    using KeyType   = std::conditional_t<kConst, const K, K>;
    using ValueType = std::conditional_t<kConst, const V, V>;
    using SizeType  = TS;
    using HashType  = THash;

    // ctor
    SKR_INLINE SparseHashMapDataRef() = default;
    SKR_INLINE SparseHashMapDataRef(PairType* ptr, SizeType index, HashType hash, bool already_exist)
        : Super(ptr, index, hash, already_exist)
    {
    }
    template <bool kConstRHS>
    SKR_INLINE SparseHashMapDataRef(const SparseHashSetDataRef<KVPair<K, V>, SizeType, HashType, kConstRHS>& rhs)
        : Super(rhs)
    {
    }
    template <bool kConstRHS>
    SKR_INLINE SparseHashMapDataRef(const SparseHashMapDataRef<K, V, SizeType, HashType, kConstRHS>& rhs)
        : Super(rhs)
    {
    }

    // getter & validator
    SKR_INLINE PairType*  ptr() const { return Super::ptr(); }
    SKR_INLINE PairType&  ref() const { return Super::ref(); }
    SKR_INLINE KeyType&   key() const { return Super::ref().key; }
    SKR_INLINE ValueType& value() const { return Super::ref().value; }
    SKR_INLINE SizeType   index() const { return Super::index(); }
    SKR_INLINE HashType   hash() const { return Super::hash(); }
    SKR_INLINE bool       already_exist() const { return Super::already_exist(); }
    SKR_INLINE bool       is_valid() const { return Super::is_valid(); }

    // operators
    SKR_INLINE explicit operator bool() { return is_valid(); }
    // SKR_INLINE T&       operator*() const { return ref(); }
    // SKR_INLINE T*       operator->() const { return ptr(); }

    // compare
    SKR_INLINE bool operator==(const SparseHashMapDataRef& rhs) const { return Super::operator==(rhs); }
    SKR_INLINE bool operator!=(const SparseHashMapDataRef& rhs) const { return Super::operator!=(rhs); }
};
} // namespace skr::container

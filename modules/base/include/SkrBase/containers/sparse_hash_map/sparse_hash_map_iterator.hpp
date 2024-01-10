#pragma once
#include "sparse_hash_map_def.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_iterator.hpp"

namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashMapCursor {
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<KVPair<K, V>, TS, THash>, TBitBlock, TS, kConst>;
    using DataType              = std::conditional_t<kConst, const KVPair<K, V>, KVPair<K, V>>;
    using KeyType               = std::conditional_t<kConst, const K, K>;
    using ValueType             = std::conditional_t<kConst, const V, V>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashMapCursor(const SparseArrayCursorType& sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashMapCursor(const SparseHashMapCursor& rhs)            = default;
    inline SparseHashMapCursor(SparseHashMapCursor&& rhs)                 = default;
    inline SparseHashMapCursor& operator=(const SparseHashMapCursor& rhs) = default;
    inline SparseHashMapCursor& operator=(SparseHashMapCursor&& rhs)      = default;

    // no factory, just wrapper of SparseArrayCursor

    // getter
    inline DataType&  ref() const { return _cursor.ref()._sparse_hash_set_data; }
    inline DataType*  ptr() const { return &_cursor.ref()._sparse_hash_set_data; }
    inline KeyType&   key() const { return ref()._key; }
    inline ValueType& value() const { return ref()._value; }
    inline THash      hash() const { return _cursor.ref()._sparse_hash_set_hash; }
    inline SizeType   index() const { return _cursor.index(); }

    // move & validator
    inline void move_next() { _cursor.move_next(); }
    inline void move_prev() { _cursor.move_prev(); }
    inline void reset_to_begin() { _cursor.reset_to_begin(); }
    inline void reset_to_end() { _cursor.reset_to_end(); }

    // reach & validate
    inline bool reach_begin() const { return _cursor.reach_begin(); }
    inline bool reach_end() const { return _cursor.reach_end(); }
    inline bool is_valid() const { return _cursor.is_valid(); }

    // compare
    inline bool operator==(const SparseHashMapCursor& rhs) const { return _cursor == rhs._cursor; }
    inline bool operator!=(const SparseHashMapCursor& rhs) const { return !(*this == rhs); }

private:
    SparseArrayCursorType _cursor;
};

template <typename K, typename V, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashMapIter {
    using CursorType            = SparseHashMapCursor<K, V, TBitBlock, TS, THash, kConst>;
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<KVPair<K, V>, TS, THash>, TBitBlock, TS, kConst>;
    using DataType              = std::conditional_t<kConst, const KVPair<K, V>, KVPair<K, V>>;
    using KeyType               = std::conditional_t<kConst, const K, K>;
    using ValueType             = std::conditional_t<kConst, const V, V>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashMapIter(SparseArrayCursorType sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashMapIter(const SparseHashMapIter& rhs)            = default;
    inline SparseHashMapIter(SparseHashMapIter&& rhs)                 = default;
    inline SparseHashMapIter& operator=(const SparseHashMapIter& rhs) = default;
    inline SparseHashMapIter& operator=(SparseHashMapIter&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline THash     hash() const { return _cursor.hash(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};

template <typename K, typename V, typename TBitBlock, typename TS, typename THash, bool kConst>
struct SparseHashMapIterInv {
    using CursorType            = SparseHashMapCursor<K, V, TBitBlock, TS, THash, kConst>;
    using SparseArrayCursorType = SparseArrayCursor<SparseHashSetData<KVPair<K, V>, TS, THash>, TBitBlock, TS, kConst>;
    using DataType              = std::conditional_t<kConst, const KVPair<K, V>, KVPair<K, V>>;
    using KeyType               = std::conditional_t<kConst, const K, K>;
    using ValueType             = std::conditional_t<kConst, const V, V>;
    using SizeType              = TS;

    // ctor & copy & move & assign & move assign
    inline SparseHashMapIterInv(SparseArrayCursorType sparse_array_cursor)
        : _cursor(sparse_array_cursor)
    {
    }
    inline SparseHashMapIterInv(const SparseHashMapIterInv& rhs)            = default;
    inline SparseHashMapIterInv(SparseHashMapIterInv&& rhs)                 = default;
    inline SparseHashMapIterInv& operator=(const SparseHashMapIterInv& rhs) = default;
    inline SparseHashMapIterInv& operator=(SparseHashMapIterInv&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline THash     hash() const { return _cursor.hash(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};

} // namespace skr::container
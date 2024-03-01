#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/memory/memory_traits.hpp"

// SparseHashSet structs
namespace skr::container
{
// SparseHashSet 的数据定义，存储于 SparseVector 中
// 存储的 hash 用于快速比较，next 用于查找 hash 链表
template <typename T, typename TS, typename HashType>
struct SparseHashSetStorage {
    T          _sparse_hash_set_data;
    HashType   _sparse_hash_set_hash;
    mutable TS _sparse_hash_set_next;
};

// SparseHashSet 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
template <typename T, typename TS, typename THash, bool kConst>
struct SparseHashSetDataRef {
    using DataType = std::conditional_t<kConst, const T, T>;
    using SizeType = TS;
    using HashType = THash;

    SKR_INLINE SparseHashSetDataRef() = default;
    SKR_INLINE SparseHashSetDataRef(DataType* ptr, SizeType index, HashType hash, bool already_exist)
        : _ptr(ptr)
        , _index(index)
        , _hash(hash)
        , _already_exist(already_exist)
    {
    }
    template <bool kConstRHS>
    SKR_INLINE SparseHashSetDataRef(const SparseHashSetDataRef<T, SizeType, HashType, kConstRHS>& rhs)
        : _ptr(const_cast<DataType*>(rhs.ptr()))
        , _index(rhs.index())
        , _hash(rhs.hash())
        , _already_exist(rhs.already_exist())
    {
    }

    // getter & validator
    SKR_INLINE DataType* ptr() const { return _ptr; }
    SKR_INLINE DataType& ref() const { return *_ptr; }
    SKR_INLINE SizeType  index() const { return _index; }
    SKR_INLINE HashType  hash() const { return _hash; }
    SKR_INLINE bool      already_exist() const { return _already_exist; }
    SKR_INLINE bool      is_valid() const { return _ptr != nullptr && _index != npos_of<SizeType>; }

    // operators
    SKR_INLINE explicit operator bool() { return is_valid(); }
    // SKR_INLINE DataRef&       operator*() const { return ref(); }
    // SKR_INLINE DataRef*       operator->() const { return ptr(); }

    // compare
    SKR_INLINE bool operator==(const SparseHashSetDataRef& rhs) const { return _ptr == rhs._ptr; }
    SKR_INLINE bool operator!=(const SparseHashSetDataRef& rhs) const { return _ptr != rhs._ptr; }

private:
    // add/emplace: 添加的元素指针
    // find: 找到的元素指针
    DataType* _ptr = nullptr;

    // add/emplace: 添加的元素下标
    // find: 找到的元素下标
    SizeType _index = npos_of<SizeType>;

    // add/emplace: 元素 hash
    // find: 元素 hash
    HashType _hash = 0;

    // add/emplace: 元素是否已经存在
    // find: false
    bool _already_exist = false;
};
} // namespace skr::container

// SparseHashSetStorage memory traits
namespace skr::memory
{
template <typename T, typename TS, typename HashType>
struct MemoryTraits<skr::container::SparseHashSetStorage<T, TS, HashType>, skr::container::SparseHashSetStorage<T, TS, HashType>> : public MemoryTraits<T, T> {
};
} // namespace skr::memory

// TODO. skr swap
namespace std
{
template <typename T, typename TS, typename HashType>
SKR_INLINE void swap(::skr::container::SparseHashSetStorage<T, TS, HashType>& a, ::skr::container::SparseHashSetStorage<T, TS, HashType>& b)
{
    ::std::swap(a._sparse_hash_set_data, b._sparse_hash_set_data);
    ::std::swap(a._sparse_hash_set_hash, b._sparse_hash_set_hash);
    ::std::swap(a._sparse_hash_set_next, b._sparse_hash_set_next);
}
} // namespace std
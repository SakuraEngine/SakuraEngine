#pragma once
#include "SkrBase/config.h"
#include "SkrBase/tools/integer_tools.hpp"

// SparseHashSet structs
namespace skr
{
// SparseHashSet 的数据定义，存储于 SparseArray 中
// 存储的 hash 用于快速比较，next 用于查找 hash 链表
template <typename T, typename TS, typename HashType>
struct SparseHashSetData {
    T          _sparse_hash_set_data;
    HashType   _sparse_hash_set_hash;
    mutable TS _sparse_hash_set_next;
};

// SparseHashSet 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
// TODO. memory traits 穿透
template <typename T, typename TS>
struct SparseHashSetDataRef {
    // add/emplace: 添加的元素指针
    // find: 找到的元素指针
    // remove: nullptr // TODO. check it
    T* data = nullptr;

    // add/emplace: 添加的元素下标
    // find: 找到的元素下标
    // remove: 移除的元素下标
    TS index = npos_of<TS>;

    // add/emplace: 元素是否已经存在
    // find: false
    // remove: false
    bool already_exist = false;

    SKR_INLINE SparseHashSetDataRef()
    {
    }
    SKR_INLINE SparseHashSetDataRef(T* data, TS index, bool already_exist = false)
        : data(data)
        , index(index)
        , already_exist(already_exist)
    {
    }
    SKR_INLINE operator bool() { return data != nullptr || index != npos_of<TS>; }
    SKR_INLINE T& operator*() const { return *data; }
    SKR_INLINE T* operator->() const { return data; }
};
} // namespace skr
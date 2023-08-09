#pragma once
#include "SkrBase/config.h"
#include "SkrBase/tools/integer_tools.hpp"

// USet structs
namespace skr
{
// USet 的数据定义，存储于 SparseArray 中
// 存储的 hash 用于快速比较，next 用于查找 hash 链表
template <typename T, typename TS, typename HashType>
struct USetData {
    T          data;
    HashType   hash;
    mutable TS next;
};

// USet 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
template <typename T, typename TS>
struct USetDataRef {
    // add/emplace: 添加的元素下标
    // find: 找到的元素下表
    // remove: 移除的元素下标
    T* data;

    // add/emplace: 添加的元素下标
    // find: 找到的元素下表
    // remove: 移除的元素下标
    TS index;

    // add/emplace: 元素是否已经存在
    // find: false
    // remove: false
    bool already_exist;

    SKR_INLINE USetDataRef()
        : data(nullptr)
        , index(npos_of<TS>)
        , already_exist(false)
    {
    }
    SKR_INLINE USetDataRef(T* data, TS index, bool already_exist = false)
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
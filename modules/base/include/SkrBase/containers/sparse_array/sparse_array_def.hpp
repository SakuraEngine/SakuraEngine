#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/array/array_def.hpp"

// SparseArray structs
namespace skr
{
// SparseArray 的 Element 定义
// 空穴状态会变为链表的节点，带来的问题是当 sizeof(T) < sizeof(TS) * 2 时，会产生不必要的浪费的浪费
// 不过通常这种浪费是可接受的
template <typename T, typename TS>
union SparseArrayData
{
    // free linked list
    struct
    {
        TS _sparse_array_freelist_prev;
        TS _sparse_array_freelist_next;
    };

    // data
    T _sparse_array_data;
};

// SparseArray 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
// 规则见 ArrayDataRef
template <typename T, typename TS>
using SparseArrayDataRef = ArrayDataRef<T, TS>;
} // namespace skr

// TODO. skr swap
namespace std
{
template <typename T, typename TS>
SKR_INLINE void swap(::skr::SparseArrayData<T, TS>& a, ::skr::SparseArrayData<T, TS>& b)
{
    ::std::swap(a._sparse_array_data, b._sparse_array_data);
}
} // namespace std
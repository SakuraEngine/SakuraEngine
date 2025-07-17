#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/vector/vector_def.hpp"
#include "SkrBase/memory/memory_traits.hpp"
#include "SkrBase/misc/swap.hpp"

// SparseVector structs
namespace skr::container
{
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4624)
#endif

// free linked list helper, for generic
template <typename TSize>
struct SparseVectorFreeListNode {
    TSize prev = 0;
    TSize next = 0;
};

// SparseVector 的 Element 定义
// 空穴状态会变为链表的节点，带来的问题是当 sizeof(T) < sizeof(TSize) * 2 时，会产生不必要的浪费的浪费
// 不过通常这种浪费是可接受的
template <typename T, typename TSize>
union SparseVectorStorage
{
    // free linked list
    struct
    {
        TSize _sparse_vector_freelist_prev;
        TSize _sparse_vector_freelist_next;
    };

    // data
    T _sparse_vector_data;
};

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// SparseVector 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
// 规则见 VectorDataRef
template <typename T, typename TSize, bool kConst>
using SparseVectorDataRef = VectorDataRef<T, TSize, kConst>;
} // namespace skr::container

// SparseVectorStorage data memory traits
namespace skr::memory
{
template <typename T, typename TSize>
struct MemoryTraits<skr::container::SparseVectorStorage<T, TSize>, skr::container::SparseVectorStorage<T, TSize>> : public MemoryTraits<T, T> {
};
} // namespace skr::memory

namespace skr
{
template <typename T, typename TSize>
struct Swap<::skr::container::SparseVectorStorage<T, TSize>> {
    static void call(::skr::container::SparseVectorStorage<T, TSize>& a, ::skr::container::SparseVectorStorage<T, TSize>& b)
    {
        Swap<T>::call(a._sparse_vector_data, b._sparse_vector_data);
    }
};
} // namespace skr
#pragma once
#include "SkrRT/base/config.hpp"
#include "functor.hpp"
#include "utils.hpp"

namespace skr::algo
{
// heap jump
template <typename T>
SKR_INLINE constexpr T heap_lchild_idx(T index) { return index * 2 + 1; }
template <typename T>
SKR_INLINE constexpr T heap_rchild_idx(T index) { return heap_lchild_idx(index) + 1; }
template <typename T>
SKR_INLINE constexpr T heap_parent_idx(T index) { return index ? (index - 1) / 2 : 0; }
template <typename T>
SKR_INLINE constexpr bool heap_is_leaf(T index, T count) { return heap_lchild_idx(index) >= count; }

// sift down
template <typename T, typename TS, typename TP = Less<>>
SKR_INLINE void heap_sift_down(T heap, TS idx, TS count, TP&& p = TP())
{
    while (!heap_is_leaf(idx, count))
    {
        const TS l_child_idx = heap_lchild_idx(idx);
        const TS r_child_idx = l_child_idx + 1;

        // find min child
        TS min_child_idx = l_child_idx;
        if (r_child_idx < count)
        {
            min_child_idx = p(*(heap + l_child_idx), *(heap + r_child_idx)) ? l_child_idx : r_child_idx;
        }

        // now element is on his location
        if (!p(*(heap + min_child_idx), *(heap + idx)))
            break;

        std::swap(*(heap + idx), *(heap + min_child_idx));
        idx = min_child_idx;
    }
}

// sift up
template <class T, typename TS, class TP = Less<>>
SKR_INLINE TS heap_sift_up(T* heap, TS root_idx, TS node_idx, TP&& p = TP())
{
    while (node_idx > root_idx)
    {
        TS parent_idx = heap_parent_idx(node_idx);

        // now element is on his location
        if (!p(*(heap + node_idx), *(heap + parent_idx)))
            break;

        std::swap(*(heap + node_idx), *(heap + parent_idx));
        node_idx = parent_idx;
    }

    return node_idx;
}

// is heap
template <typename T, typename TS, typename TP = Less<>>
SKR_INLINE bool is_heap(T* heap, TS count, TP&& p = TP())
{
    for (TS i = 1; i < count; ++i)
    {
        if (p(*(heap + i), *(heap + heapParentIdx(i))))
            return false;
    }
    return true;
}

// heapify
template <typename T, typename TS, typename TP = Less<>>
SKR_INLINE void heapify(T* heap, TS count, TP&& p = TP())
{
    if (count > 1)
    {
        TS idx = heap_parent_idx(count - 1);
        while (true)
        {
            heap_sift_down(heap, idx, count, std::forward<TP>(p));
            if (idx == 0)
                return;
            --idx;
        }
    }
}

// heap sort
template <typename T, typename TS, class TP = Less<>>
SKR_INLINE void heap_sort(T heap, TS count, TP&& p = TP())
{
    auto reverse_pred = [&](const auto& a, const auto& b) -> bool { return !p(a, b); };

    // use reverse_pred heapify, and pop head swap to tail
    heapify(heap, count, reverse_pred);

    for (TS cur_count = count - 1; cur_count > 0; --cur_count)
    {
        std::swap(*heap, *(heap + cur_count));
        heap_sift_down(heap, (TS)0, cur_count, reverse_pred);
    }
}
} // namespace skr::algo
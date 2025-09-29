#pragma once
#include <SkrContainersDef/sparse_vector.hpp>
#include <SkrObject/object.hpp>

namespace skr
{
// TODO. 先实现一个最简单的 Mark-Sweep，后续再做优化
struct ObjectArrayData
{
    Object* object;
    uint64_t generation_id;
    bool is_garage;    // 是否是垃圾，已经进入死亡队列
    bool is_reachable; // 扫描阶段标记
};
struct ObjectRootData
{
    Object* object;
    uint64_t root_count;
};

struct ObjectManager
{
    // object global
    static SparseVector<ObjectArrayData>& g_object_array();
    static Vector<ObjectRootData>& g_object_roots();
    static std::atomic<uint64_t>& g_generation();
    static Vector<uint64_t>& g_garbage_indices();

    // object operations
    static void register_object(Object* obj);
    static Object* try_solve_object(uint64_t index, uint64_t generation);
    static void root_add(Object* obj);
    static void root_release(Object* obj);
    static void get_object_index_and_generation(Object* obj, uint64_t& index, uint64_t& generation);

    // gc
    static void gc();

private:
    // helpers
    static void _mark_visible_recursive(Object* obj);
};

} // namespace skr
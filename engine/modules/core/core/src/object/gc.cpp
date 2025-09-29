#include <SkrObject/gc.hpp>
#include <SkrObject/object_ptr.hpp>
#include "./gc_private.hpp"

// object manager
namespace skr
{
// object global
SparseVector<ObjectArrayData>& ObjectManager::g_object_array()
{
    static SparseVector<ObjectArrayData> g_objects;
    return g_objects;
}
Vector<ObjectRootData>& ObjectManager::g_object_roots()
{
    static Vector<ObjectRootData> g_roots;
    return g_roots;
}
std::atomic<uint64_t>& ObjectManager::g_generation()
{
    static std::atomic<uint64_t> g_generation = 0;
    return g_generation;
}
Vector<uint64_t>& ObjectManager::g_garbage_indices()
{
    static Vector<uint64_t> g_garbages;
    return g_garbages;
}

// object registry
void ObjectManager::register_object(Object* obj)
{
    auto& obj_arr = g_object_array();
    auto generation = g_generation().fetch_add(1, std::memory_order_relaxed);
    auto add_result = obj_arr.add_unsafe();

    // setup object index
    obj->_object_array_index = add_result.index();

    // setup object array data
    add_result.ref().object = obj;
    add_result.ref().generation_id = generation;
    add_result.ref().is_garage = false;
    add_result.ref().is_reachable = true; // new object is always reachable
}
Object* ObjectManager::try_solve_object(uint64_t index, uint64_t generation)
{
    auto& obj_arr = g_object_array();
    if (!obj_arr.is_valid_index(index)) return nullptr;
    if (!obj_arr.has_data(index)) return nullptr;
    auto& obj_data = obj_arr[index];
    if (obj_data.generation_id != generation) return nullptr;
    return obj_data.object;
}
void ObjectManager::root_add(Object* obj)
{
    auto& root_arr = g_object_roots();
    if (auto found = root_arr.find_if([obj](const ObjectRootData& r) { return r.object == obj; }))
    {
        found.ref().root_count++;
        return;
    }
    else
    {
        auto add_result = root_arr.add_unsafe();
        add_result.ref().object = obj;
        add_result.ref().root_count = 1;
    }
}
void ObjectManager::root_release(Object* obj)
{
    auto& root_arr = g_object_roots();
    if (auto found = root_arr.find_if([obj](const ObjectRootData& r) { return r.object == obj; }))
    {
        found.ref().root_count--;
        if (found.ref().root_count == 0)
            root_arr.remove_at(found.index());
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
}

// gc
void ObjectManager::gc()
{
    auto& obj_arr = g_object_array();
    auto& root_arr = g_object_roots();
    auto& garbage_arr = g_garbage_indices();

    // step 1. clear reachable flag
    for (auto& obj_data : obj_arr)
    {
        obj_data.is_reachable = false;
    }

    // step 2. mark from roots
    for (auto& root : root_arr)
    {
        _mark_visible_recursive(root.object);
    }

    // step 3. collect garbage
    for (auto& obj_data : obj_arr)
    {
        if (!obj_data.is_reachable && !obj_data.is_garage)
        {
            obj_data.is_garage = true;
            garbage_arr.push_back(obj_data.object->_object_array_index);
        }
    }

    // step 4. delete garbage
    for (auto index : garbage_arr)
    {
        SKR_ASSERT(obj_arr.is_valid_index(index));
        SKR_ASSERT(obj_arr.has_data(index));

        auto& obj_data = obj_arr[index];

        SKR_ASSERT(obj_data.is_garage);
        SKR_ASSERT(!obj_data.is_reachable);

        SkrDelete(obj_data.object);

        obj_arr.remove_at(index);
    }
    garbage_arr.clear();
}
void ObjectManager::get_object_index_and_generation(Object* obj, uint64_t& index, uint64_t& generation)
{
    auto& obj_arr = g_object_array();
    index = obj->_object_array_index;
    SKR_ASSERT(obj_arr.is_valid_index(index));
    SKR_ASSERT(obj_arr.has_data(index));
    generation = obj_arr[index].generation_id;
}

// helpers
void ObjectManager::_mark_visible_recursive(Object* obj)
{
    auto& obj_arr = ObjectManager::g_object_array();
    auto& obj_data = obj_arr[obj->_object_array_index];

    // fast exit
    if (obj_data.is_reachable || obj_data.is_garage) return;

    // mark self reachable
    obj_data.is_reachable = true;

    // scan children
    obj->object_scan_gc([&](ObjPtrBasic& child_ptr) {
        if (child_ptr.is_object())
        {
            _mark_visible_recursive(child_ptr.get_object());
        }
        return true;
    });
}
} // namespace skr
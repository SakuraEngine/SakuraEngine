#include "SkrGraphics/api.h"
#include "SkrGraphics/containers.hpp"
#ifdef CGPU_USE_VULKAN
    // #include "SkrGraphics/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
    // #include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"
#endif
#ifdef CGPU_USE_METAL
    // #include "SkrGraphics/backend/metal/cgpu_metal.h"
#endif
#include "common_utils.h"
#include <functional>

// Runtime Table
struct CGPURuntimeTable {
    struct CreatedQueue {
        CGPUDeviceId device;
        union
        {
            uint64_t type_index;
            struct
            {
                ECGPUQueueType type;
                uint32_t index;
            };
        };
        CGPUQueueId queue;
        bool operator==(const CreatedQueue& rhs) const
        {
            return device == rhs.device && type_index == rhs.type_index;
        }
    };
    CGPUQueueId TryFindQueue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
    {
        CreatedQueue to_find = {};
        to_find.device = device;
        to_find.type = type;
        to_find.index = index;
        if (auto found = created_queues.find(to_find);found.is_valid())
            return found.ref().queue;
        return nullptr;
    }
    void AddNewQueue(CGPUQueueId queue, ECGPUQueueType type, uint32_t index)
    {
        CreatedQueue new_queue = {};
        new_queue.device = queue->device;
        new_queue.type = type;
        new_queue.index = index;
        new_queue.queue = queue;
        created_queues.add(new_queue);
    }
    void early_sweep()
    {
        for (auto [name, callback] : custom_early_sweep_callbacks)
        {
            if (custom_data_map.find(name) != custom_data_map.end())
            {
                callback();
            }
        }
    }
    ~CGPURuntimeTable()
    {
        for (auto [name, callback] : custom_sweep_callbacks)
        {
            if (custom_data_map.find(name) != custom_data_map.end())
            {
                callback();
            }
        }
    }
    cgpu::Vector<CreatedQueue> created_queues;
    // TODO: replace with skr::hash_map
    cgpu::FlatHashMap<cgpu::stl_u8string, void*> custom_data_map;
    cgpu::FlatHashMap<cgpu::stl_u8string, std::function<void()>> custom_sweep_callbacks;
    cgpu::FlatHashMap<cgpu::stl_u8string, std::function<void()>> custom_early_sweep_callbacks;
};

struct CGPURuntimeTable* cgpu_create_runtime_table()
{
    return SkrNew<CGPURuntimeTable>();
}

void cgpu_early_free_runtime_table(struct CGPURuntimeTable* table)
{
    table->early_sweep();
}

void cgpu_free_runtime_table(struct CGPURuntimeTable* table)
{
    SkrDelete(table);
}

void cgpu_runtime_table_add_queue(CGPUQueueId queue, ECGPUQueueType type, uint32_t index)
{
    queue->device->adapter->instance->runtime_table->AddNewQueue(queue, type, index);
}

CGPUQueueId cgpu_runtime_table_try_get_queue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    return device->adapter->instance->runtime_table->TryFindQueue(device, type, index);
}

void cgpu_runtime_table_add_custom_data(struct CGPURuntimeTable* table, const char8_t* key, void* data)
{
    table->custom_data_map[key] = data;
}

void cgpu_runtime_table_add_sweep_callback(struct CGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata)
{
    table->custom_sweep_callbacks[key] = [=](){
        pfn(usrdata);
    };
}

void cgpu_runtime_table_add_early_sweep_callback(struct CGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata)
{
    table->custom_early_sweep_callbacks[key] = [=](){
        pfn(usrdata);
    };
}

void* cgpu_runtime_table_try_get_custom_data(struct CGPURuntimeTable* table, const char8_t* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end())
    {
        return table->custom_data_map[key];
    }
    return nullptr;
}

bool cgpu_runtime_table_remove_custom_data(struct CGPURuntimeTable* table, const char8_t* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end())
    {
        return table->custom_data_map.erase(key);
    }
    return false;
}

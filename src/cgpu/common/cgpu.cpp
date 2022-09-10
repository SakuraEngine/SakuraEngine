#include "EASTL/unordered_map.h"
#include "platform/shared_library.h"
#include "cgpu/api.h"
#ifdef CGPU_USE_VULKAN
    #include "cgpu/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
    #include "cgpu/backend/d3d12/cgpu_d3d12.h"
#endif
#ifdef CGPU_USE_METAL
    #include "cgpu/backend/metal/cgpu_metal.h"
#endif
#include "common_utils.h"
#include <EASTL/string_map.h>
#include <EASTL/vector.h>

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
        bool operator==(const CreatedQueue& rhs)
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
        const auto& found = eastl::find(
        created_queues.begin(), created_queues.end(), to_find);
        if (found != created_queues.end())
        {
            return found->queue;
        }
        return nullptr;
    }
    void AddNewQueue(CGPUQueueId queue, ECGPUQueueType type, uint32_t index)
    {
        CreatedQueue new_queue = {};
        new_queue.device = queue->device;
        new_queue.type = type;
        new_queue.index = index;
        new_queue.queue = queue;
        created_queues.push_back(new_queue);
    }
    eastl::vector<CreatedQueue> created_queues;
    eastl::string_map<void*> custom_data_map;
};

struct CGPURuntimeTable* cgpu_create_runtime_table()
{
    return new CGPURuntimeTable();
}

void cgpu_free_runtime_table(struct CGPURuntimeTable* table)
{
    delete table;
}

void cgpu_runtime_table_add_queue(CGPUQueueId queue, ECGPUQueueType type, uint32_t index)
{
    queue->device->adapter->instance->runtime_table->AddNewQueue(queue, type, index);
}

CGPUQueueId cgpu_runtime_table_try_get_queue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    return device->adapter->instance->runtime_table->TryFindQueue(device, type, index);
}

void cgpu_runtime_table_add_custom_data(struct CGPURuntimeTable* table, const char* key, void* data)
{
    table->custom_data_map[key] = data;
}

void* cgpu_runtime_table_try_get_custom_data(struct CGPURuntimeTable* table, const char* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end())
    {
        return table->custom_data_map[key];
    }
    return nullptr;
}

bool cgpu_runtime_table_remove_custom_data(struct CGPURuntimeTable* table, const char* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end())
    {
        return table->custom_data_map.erase(key);
    }
    return false;
}

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
#include "cgpu/drivers/cgpu_ags.h"
#include "cgpu/drivers/cgpu_nvapi.h"
#include "runtime_table.h"
#include <EASTL/vector.h>

// AGS
#if defined(AMDAGS)
static AGSContext* pAgsContext = NULL;
static AGSGPUInfo gAgsGpuInfo = {};
    // Actually it's always windows.
    #if defined(_WIN64)
        #pragma comment(lib, "amd_ags_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "amd_ags_x86.lib")
    #endif
#endif

ECGpuAGSReturnCode cgpu_ags_init(struct CGpuInstance* Inst)
{
#if defined(AMDAGS)
    AGSConfiguration config = {};
    int apiVersion = AGS_MAKE_VERSION(6, 0, 1);
    auto Status = agsInitialize(apiVersion, &config, &pAgsContext, &gAgsGpuInfo);
    Inst->ags_status = (ECGpuAGSReturnCode)Status;
    return Inst->ags_status;
#else
    return CGPU_AGS_NONE;
#endif
}

void cgpu_ags_exit()
{
#if defined(AMDAGS)
    agsDeInitialize(pAgsContext);
#endif
}

// NVAPI
#if defined(NVAPI)
    #if defined(_WIN64)
        #pragma comment(lib, "nvapi_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "nvapi_x86.lib")
    #endif
#endif
ECGpuNvAPI_Status cgpu_nvapi_init(struct CGpuInstance* Inst)
{
#if defined(NVAPI)
    auto Status = NvAPI_Initialize();
    Inst->nvapi_status = (ECGpuNvAPI_Status)Status;
    return Inst->nvapi_status;
#else
    return ECGpuNvAPI_Status::CGPU_NVAPI_NONE;
#endif
}
void cgpu_nvapi_exit()
{
#if defined(NVAPI)
    NvAPI_Unload();
#endif
}

// Runtime Table
class CGpuRuntimeTable
{
public:
    struct CreatedQueue {
        CGpuDeviceId device;
        union
        {
            uint64_t type_index;
            struct
            {
                ECGpuQueueType type;
                uint32_t index;
            };
        };
        CGpuQueueId queue;
        bool operator==(const CreatedQueue& rhs)
        {
            return device == rhs.device && type_index == rhs.type_index;
        }
    };
    CGpuQueueId TryFindQueue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
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
    void AddNewQueue(CGpuQueueId queue, ECGpuQueueType type, uint32_t index)
    {
        CreatedQueue new_queue = {};
        new_queue.device = queue->device;
        new_queue.type = type;
        new_queue.index = index;
        new_queue.queue = queue;
        created_queues.push_back(new_queue);
    }
    eastl::vector<CreatedQueue> created_queues;
};

struct CGpuRuntimeTable* cgpu_create_runtime_table()
{
    return new CGpuRuntimeTable();
}

void cgpu_free_runtime_table(struct CGpuRuntimeTable* table)
{
    delete table;
}

void cgpu_runtime_table_add_queue(CGpuQueueId queue, ECGpuQueueType type, uint32_t index)
{
    queue->device->adapter->instance->runtime_table->AddNewQueue(queue, type, index);
}

CGpuQueueId cgpu_runtime_table_try_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    return device->adapter->instance->runtime_table->TryFindQueue(device, type, index);
}
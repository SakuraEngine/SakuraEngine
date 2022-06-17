#include "EASTL/unordered_map.h"
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
#include "common_utils.h"
#include <EASTL/vector.h>

// AGS
#if defined(AMDAGS)
static AGSContext* pAgsContext = NULL;
static AGSGPUInfo gAgsGpuInfo = {};
static uint32_t driverVersion = 0;
    // Actually it's always windows.
    #if defined(_WIN64)
        #pragma comment(lib, "amd_ags_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "amd_ags_x86.lib")
    #endif
#endif

ECGPUAGSReturnCode cgpu_ags_init(struct CGPUInstance* Inst)
{
#if defined(AMDAGS)
    AGSConfiguration config = {};
    int apiVersion = AGS_MAKE_VERSION(6, 0, 1);
    auto Status = agsInitialize(apiVersion, &config, &pAgsContext, &gAgsGpuInfo);
    Inst->ags_status = (ECGPUAGSReturnCode)Status;
    if (Status == AGS_SUCCESS)
    {
        char* stopstring;
        driverVersion = strtoul(gAgsGpuInfo.driverVersion, &stopstring, 10);
    }
    return Inst->ags_status;
#else
    return CGPU_AGS_NONE;
#endif
}

uint32_t cgpu_ags_get_driver_version()
{
#if defined(AMDAGS)
    return driverVersion;
#endif
    return 0;
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
ECGPUNvAPI_Status cgpu_nvapi_init(struct CGPUInstance* Inst)
{
#if defined(NVAPI)
    auto Status = NvAPI_Initialize();
    Inst->nvapi_status = (ECGPUNvAPI_Status)Status;
    return Inst->nvapi_status;
#else
    return ECGPUNvAPI_Status::CGPU_NVAPI_NONE;
#endif
}

uint32_t cgpu_nvapi_get_driver_version()
{
#if defined(NVAPI)
    NvU32 v = 0;         // version
    NvAPI_ShortString b; // branch
    auto Status = NvAPI_SYS_GetDriverAndBranchVersion(&v, b);
    if (Status != NVAPI_OK)
    {
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(Status, string);
        cgpu_warn("[warn] nvapi failed to get driver version! \n message: %s", string);
        return v;
    }
    return v;
#endif
    return 0;
}

uint64_t cgpu_nvapi_d3d12_query_cpu_visible_vram(struct ID3D12Device* Device)
{
#if defined(NVAPI)
    NvU64 total, budget;
    NvAPI_D3D12_QueryCpuVisibleVidmem(Device, &total, &budget);
    return budget;
#endif
    return 0;
}

void cgpu_nvapi_exit()
{
#if defined(NVAPI)
    NvAPI_Unload();
#endif
}

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
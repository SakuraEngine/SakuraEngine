#include "cgpu/api.h"
#ifdef CGPU_USE_VULKAN
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#endif

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #define _MACOS
    #endif
#elif defined _WIN32 || defined _WIN64
#endif 
#include "assert.h"

CGpuInstanceId cgpu_create_instance(const CGpuInstanceDescriptor* desc)
{
    assert((desc->backend == ECGPUBackEnd_VULKAN
          || desc->backend == ECGPUBackEnd_D3D12) 
        && "cgpu support only vulkan & d3d12 currently!");
    const CGpuProcTable* tbl = CGPU_NULLPTR;
    const CGpuSurfacesProcTable* s_tbl = CGPU_NULLPTR;

    if (desc->backend == ECGPUBackEnd_COUNT)
    {

    }
#ifdef CGPU_USE_VULKAN
    else if (desc->backend == ECGPUBackEnd_VULKAN) {
        tbl = CGPU_VulkanProcTable();
        s_tbl = CGPU_VulkanSurfacesProcTable();
    } 
#endif
#ifdef CGPU_USE_D3D12
    else if (desc->backend == ECGPUBackEnd_D3D12) {
        tbl = CGPU_D3D12ProcTable();
        s_tbl = CGPU_D3D12SurfacesProcTable();
    }
#endif
    CGpuInstance* instance = (CGpuInstance*)tbl->create_instance(desc);
    instance->proc_table = tbl;
    instance->surfaces_table = s_tbl;
    return instance;
}

RUNTIME_API void cgpu_query_instance_features(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");
    
    instance->proc_table->query_instance_features(instance, features);
}

void cgpu_free_instance(CGpuInstanceId instance)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->enum_adapters && "free_instance Proc Missing!");

    instance->proc_table->free_instance(instance);
}

void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

    instance->proc_table->enum_adapters(instance, adapters, adapters_num);
    // ++ proc_table_cache
    if(adapters != CGPU_NULLPTR)
    {
        for(uint32_t i = 0; i < *adapters_num; i++)
        {
            *(const CGpuProcTable**)&adapters[i]->proc_table_cache = instance->proc_table;
        }
    }
    // -- proc_table_cache
}

void cgpu_query_adapter_detail(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    adapter->proc_table_cache->query_adapter_detail(adapter, detail);
    return;
}

uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");
    
    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

    CGpuDeviceId device = adapter->proc_table_cache->create_device(adapter, desc);
    // ++ proc_table_cache
    if(device != CGPU_NULLPTR)
    {
        *(const CGpuProcTable**)&device->proc_table_cache = adapter->proc_table_cache;
    }
    // -- proc_table_cache
    return device;
}

void cgpu_free_device(CGpuDeviceId device)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGpuQueue* queue = (CGpuQueue*)device->proc_table_cache->get_queue(device, type, index);
    queue->index = index; queue->type = type; queue->device = device;
    return queue;
}

void cgpu_free_queue(CGpuQueueId queue)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->proc_table_cache->free_device && "free_device Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool(CGpuQueueId queue,
    const CGpuCommandPoolDescriptor* desc)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGpuCommandPool* encoder = (CGpuCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    encoder->queue = queue;
    return encoder;
}

RUNTIME_API void cgpu_free_command_pool(CGpuCommandPoolId encoder)
{
    assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    assert(encoder->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(encoder->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(encoder->queue->device->proc_table_cache->free_device && "free_device Proc Missing!");

    encoder->queue->device->proc_table_cache->free_command_pool(encoder);
    return;
}

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor *desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

    CGPUProcCreateShaderLibrary fn_create_shader_library = device->proc_table_cache->create_shader_library;
    CGpuShaderLibrary* shader = (CGpuShaderLibrary*)fn_create_shader_library(device, desc);
    shader->device = device;
    // handle name string
    const size_t str_len = strlen(desc->name);
    const size_t str_size = str_len + 1;
    *(void**)&shader->name = cgpu_calloc(str_size, sizeof(char8_t));
    memcpy((void*)shader->name, desc->name, str_size);
    return shader;
}

void cgpu_free_shader_library(CGpuShaderLibraryId library)
{
    assert(library != CGPU_NULLPTR && "fatal: call on NULL shader_module!");
    const CGpuDeviceId device = library->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    cgpu_free((void*)library->name);

    CGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->presentQueues == CGPU_NULLPTR) {
        assert(desc->presentQueuesCount <= 0 &&
            "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    } else {
        assert(desc->presentQueuesCount > 0 && 
            "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    CGpuSwapChain* swapchain = (CGpuSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    return swapchain;
}

void cgpu_free_swapchain(CGpuSwapChainId swapchain)
{
    assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
    return;
}


// surfaces
#if defined(_WIN32) || defined(_WIN64)
    CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window)
    {
        assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
        assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
        assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
        assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
        assert(device->adapter->instance->surfaces_table->from_hwnd != CGPU_NULLPTR && "free_instance Proc Missing!");

        return device->adapter->instance->surfaces_table->from_hwnd(device, window);
    }
#elif defined(_MACOS)
    CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, NSView* window)
    {
        assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
        assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
        assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
        assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
        assert(device->adapter->instance->surfaces_table->from_ns_view != CGPU_NULLPTR && "free_instance Proc Missing!");

        return device->adapter->instance->surfaces_table->from_ns_view(device, window);
    }
#endif

void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    assert(device->adapter->instance->surfaces_table->free_surface != CGPU_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}
//
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
}

void cgpu_query_adapter_detail(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(adapter->instance->proc_table->query_adapter_detail && "query_adapter_detail Proc Missing!");

    adapter->instance->proc_table->query_adapter_detail(adapter, detail);
    return;
}

uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(adapter->instance->proc_table->query_queue_count && "query_queue_count Proc Missing!");
    
    return adapter->instance->proc_table->query_queue_count(adapter, type);
}

CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(adapter->instance->proc_table->create_device && "create_device Proc Missing!");

    return adapter->instance->proc_table->create_device(adapter, desc);
}

void cgpu_free_device(CGpuDeviceId device)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(device->adapter->instance->proc_table->free_device && "free_device Proc Missing!");

    device->adapter->instance->proc_table->free_device(device);
    return;
}

CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(device->adapter->instance->proc_table->free_device && "free_device Proc Missing!");

    CGpuQueue* queue = (CGpuQueue*)device->adapter->instance->proc_table->get_queue(device, type, index);
    queue->index = index; queue->type = type; queue->device = device;
    return queue;
}

void cgpu_free_queue(CGpuQueueId queue)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(queue->device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(queue->device->adapter->instance->proc_table->free_device && "free_device Proc Missing!");

    queue->device->adapter->instance->proc_table->free_queue(queue);
    return;
}

RUNTIME_API CGpuCommandEncoderId cgpu_create_command_encoder(CGpuQueueId queue,
    const CGpuCommandEncoderDescriptor* desc)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(queue->device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(queue->device->adapter->instance->proc_table->free_device && "free_device Proc Missing!");

    CGpuCommandEncoder* encoder = (CGpuCommandEncoder*)queue->device->adapter->instance->proc_table->create_command_encoder(queue, desc);
    encoder->queue = queue;
    return encoder;
}

RUNTIME_API void cgpu_free_command_encoder(CGpuCommandEncoderId encoder)
{
    assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    assert(encoder->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(encoder->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(encoder->queue->device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(encoder->queue->device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(encoder->queue->device->adapter->instance->proc_table->free_device && "free_device Proc Missing!");

    encoder->queue->device->adapter->instance->proc_table->free_command_encoder(encoder);
    return;
}

// Shader APIs
CGpuShaderModuleId cgpu_create_shader_module(CGpuDeviceId device, const struct CGpuShaderModuleDescriptor *desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(device->adapter->instance->proc_table->create_shader_module && "create_shader_module Proc Missing!");

    CGPUProcCreateShaderModule fn_create_shader_module = device->adapter->instance->proc_table->create_shader_module;
    CGpuShaderModule* shader = (CGpuShaderModule*)fn_create_shader_module(device, desc);
    shader->name = desc->name;
    return shader;
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(device->adapter->instance->proc_table->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->presentQueues == CGPU_NULLPTR) {
        assert(desc->presentQueuesCount <= 0 &&
            "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    } else {
        assert(desc->presentQueuesCount > 0 && 
            "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    CGpuSwapChain* swapchain = (CGpuSwapChain*)device->adapter->instance->proc_table->create_swapchain(device, desc);
    assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    return swapchain;
}

void cgpu_free_swapchain(CGpuSwapChainId swapchain)
{
    assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(swapchain->device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(swapchain->device->adapter->instance != CGPU_NULLPTR && "fatal: Missing instance of adapter!");
    assert(swapchain->device->adapter->instance->proc_table->create_swapchain && "create_swapchain Proc Missing!");

    swapchain->device->adapter->instance->proc_table->free_swapchain(swapchain);
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
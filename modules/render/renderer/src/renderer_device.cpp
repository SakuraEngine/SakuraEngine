#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/map.hpp"
#include "SkrGraphics/dstorage.h"
#include "SkrGraphics/extensions/cgpu_nsight.h"
#include "SkrRT/io/vram_io.hpp"
#include "SkrRenderer/render_device.h"

namespace skr
{
struct SKR_RENDERER_API RendererDeviceImpl : public RendererDevice {
    friend class ::SkrRendererModule;
    void initialize(const Builder& builder) override;
    void finalize() override;

    void            create_api_objects(const Builder& builder) override;

    CGPUDeviceId get_cgpu_device() const override
    {
        return device;
    }

    ECGPUBackend get_backend() const override
    {
        return device->adapter->instance->backend;
    }

    CGPUQueueId get_gfx_queue() const override
    {
        return gfx_queue;
    }

    CGPUQueueId get_cpy_queue(uint32_t idx = 0) const override
    {
        if (idx < cpy_queues.size())
            return cpy_queues[idx];
        return cpy_queues[0];
    }

    CGPUQueueId get_compute_queue(uint32_t idx = 0) const override
    {
        if (idx < cmpt_queues.size())
            return cmpt_queues[idx];
        return cmpt_queues[0];
    }

    CGPUDStorageQueueId get_file_dstorage_queue() const override
    {
        return file_dstorage_queue;
    }

    CGPUDStorageQueueId get_memory_dstorage_queue() const override
    {
        return memory_dstorage_queue;
    }

    CGPUSamplerId get_linear_sampler() const override
    {
        return linear_sampler;
    }

    CGPURootSignaturePoolId get_root_signature_pool() const override
    {
        return root_signature_pool;
    }

protected:
    // Device objects
    uint32_t                                  backbuffer_index = 0;
    CGPUInstanceId                            instance  = nullptr;
    CGPUAdapterId                             adapter   = nullptr;
    CGPUDeviceId                              device    = nullptr;
    CGPUQueueId                               gfx_queue = nullptr;
    skr::Vector<CGPUQueueId>                  cpy_queues;
    skr::Vector<CGPUQueueId>                  cmpt_queues;
    CGPUSamplerId                             linear_sampler        = nullptr;
    CGPUDStorageQueueId                       file_dstorage_queue   = nullptr;
    CGPUDStorageQueueId                       memory_dstorage_queue = nullptr;
    CGPURootSignaturePoolId                   root_signature_pool   = nullptr;
    CGPUNSightTrackerId                       nsight_tracker        = nullptr;
};

RendererDevice* RendererDevice::Create() SKR_NOEXCEPT
{
    return SkrNew<RendererDeviceImpl>();
}

void RendererDevice::Free(RendererDevice* device) SKR_NOEXCEPT
{
    SkrDelete(device);
}

void RendererDeviceImpl::initialize(const Builder& builder)
{
    create_api_objects(builder);
}

void RendererDeviceImpl::finalize()
{
    // free dstorage queues
    if (file_dstorage_queue)
        cgpu_free_dstorage_queue(file_dstorage_queue);
    if (memory_dstorage_queue)
        cgpu_free_dstorage_queue(memory_dstorage_queue);

    cgpu_free_sampler(linear_sampler);

    // free queues & device
    for (auto& cpy_queue : cpy_queues)
    {
        if (cpy_queue && cpy_queue != gfx_queue)
            cgpu_free_queue(cpy_queue);
    }
    for (auto& cmpt_queue : cmpt_queues)
    {
        if (cmpt_queue && cmpt_queue != gfx_queue)
            cgpu_free_queue(cmpt_queue);
    }
    cpy_queues.clear();
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    // free nsight tracker
    if (nsight_tracker)
        cgpu_free_nsight_tracker(nsight_tracker);
    cgpu_free_instance(instance);
}

#define MAX_CPY_QUEUE_COUNT 2
#define MAX_CMPT_QUEUE_COUNT 2
void RendererDeviceImpl::create_api_objects(const Builder& builder)
{
    // create instance
    CGPUInstanceDescriptor instance_desc      = {};
    instance_desc.backend                     = builder.backend;
    instance_desc.enable_debug_layer          = builder.enable_debug_layer;
    instance_desc.enable_gpu_based_validation = builder.enable_gpu_based_validation;
    instance_desc.enable_set_name             = builder.enable_set_name;
    instance                                  = cgpu_create_instance(&instance_desc);

    // filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // create default nsight tracker on nvidia devices
    if (cgpux_adapter_is_nvidia(adapter))
    {
        CGPUNSightTrackerDescriptor desc = {};
        nsight_tracker                   = cgpu_create_nsight_tracker(instance, &desc);
    }

    // create device
    const auto                            cpy_queue_count_  = cgpu_min(cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER), MAX_CPY_QUEUE_COUNT);
    const auto                            cmpt_queue_count_ = cgpu_min(cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_COMPUTE), MAX_CMPT_QUEUE_COUNT);
    skr::Vector<CGPUQueueGroupDescriptor> Gs;
    auto&                                 GfxDesc = Gs.add_default().ref();
    GfxDesc.queue_type                            = CGPU_QUEUE_TYPE_GRAPHICS;
    GfxDesc.queue_count                           = 1;
    if (cpy_queue_count_)
    {
        auto& CpyDesc       = Gs.add_default().ref();
        CpyDesc.queue_type  = CGPU_QUEUE_TYPE_TRANSFER;
        CpyDesc.queue_count = cpy_queue_count_;
    }
    if (cmpt_queue_count_)
    {
        auto& CmptDesc       = Gs.add_default().ref();
        CmptDesc.queue_type  = CGPU_QUEUE_TYPE_COMPUTE;
        CmptDesc.queue_count = cmpt_queue_count_;
    }

    CGPUDeviceDescriptor device_desc = {};
    device_desc.queue_groups         = Gs.data();
    device_desc.queue_group_count    = (uint32_t)Gs.size();
    device                           = cgpu_create_device(adapter, &device_desc);
    gfx_queue                        = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    if (cpy_queue_count_) // request at least one copy queue by default
    {
        cpy_queues.resize_zeroed(cpy_queue_count_);
        for (uint32_t i = 0; i < cpy_queues.size(); i++)
        {
            cpy_queues[i] = cgpu_get_queue(device, CGPU_QUEUE_TYPE_TRANSFER, i);
        }
    }
    else // fallback: request only one graphics queue
    {
        cpy_queues.add(gfx_queue);
    }

    if (cmpt_queue_count_) // request at least one copy queue by default
    {
        cmpt_queues.resize_zeroed(cmpt_queue_count_);
        for (uint32_t i = 0; i < cmpt_queues.size(); i++)
        {
            cmpt_queues[i] = cgpu_get_queue(device, CGPU_QUEUE_TYPE_COMPUTE, i);
        }
    }
    else // fallback: request only one graphics queue
    {
        cmpt_queues.add(gfx_queue);
    }

    // dstorage queue
    auto       dstorage_cap         = cgpu_query_dstorage_availability(device);
    const bool supportDirectStorage = (dstorage_cap != SKR_DSTORAGE_AVAILABILITY_NONE);
    if (supportDirectStorage)
    {
#ifdef _WIN32
        skr_win_dstorage_set_staging_buffer_size(4096 * 4096 * 8);
#endif
        {
            auto queue_desc     = make_zeroed<CGPUDStorageQueueDescriptor>();
            queue_desc.name     = u8"DirectStorageFileQueue";
            queue_desc.capacity = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
            queue_desc.source   = SKR_DSTORAGE_SOURCE_FILE;
            queue_desc.priority = SKR_DSTORAGE_PRIORITY_NORMAL;
            file_dstorage_queue = cgpu_create_dstorage_queue(device, &queue_desc);
        }
        {
            auto queue_desc       = make_zeroed<CGPUDStorageQueueDescriptor>();
            queue_desc.name       = u8"DirectStorageMemoryQueue";
            queue_desc.capacity   = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
            queue_desc.source     = SKR_DSTORAGE_SOURCE_MEMORY;
            queue_desc.priority   = SKR_DSTORAGE_PRIORITY_NORMAL;
            memory_dstorage_queue = cgpu_create_dstorage_queue(device, &queue_desc);
        }
    }

    // create default linear sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode           = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func          = CGPU_CMP_NEVER;
    linear_sampler                     = cgpu_create_sampler(device, &sampler_desc);
}
} // namespace skr
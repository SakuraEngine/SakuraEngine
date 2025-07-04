#include "SkrGraphics/api.h"
#include "SkrTestFramework/framework.hpp"
#include <vector>

template <ECGPUBackend backend>
class QueueOperations
{
protected:
    QueueOperations()
    {
        SKR_DECLARE_ZERO(CGPUInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = false;
        desc.enable_gpu_based_validation = false;
        desc.enable_set_name = true;
        instance = cgpu_create_instance(&desc);
        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGPUAdapterId> adapters;
        adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        for (auto a : adapters)
        {
            auto gQueue = cgpu_query_queue_count(a, CGPU_QUEUE_TYPE_GRAPHICS);
            auto cQueue = cgpu_query_queue_count(a, CGPU_QUEUE_TYPE_COMPUTE);
            auto tQueue = cgpu_query_queue_count(a, CGPU_QUEUE_TYPE_TRANSFER);

            std::vector<CGPUQueueGroupDescriptor> queueGroup;
            if (gQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_GRAPHICS, 1 });
            if (cQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_COMPUTE, 1 });
            if (tQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_TRANSFER, 1 });
            SKR_DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
            descriptor.queue_groups = queueGroup.data();
            descriptor.queue_group_count = (uint32_t)queueGroup.size();

            device = cgpu_create_device(a, &descriptor);
            adapter = a;
            EXPECT_NE(device, nullptr);
            EXPECT_NE(device, CGPU_NULLPTR);
            break;
        }
    }

    ~QueueOperations()
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    void test_all();

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
};

template <ECGPUBackend backend>
void QueueOperations<backend>::test_all()
{
        SUBCASE("GetGraphicsQueue")
    {
        CGPUQueueId graphicsQueue;
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        if (gQueue > 0)
        {
            graphicsQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
            EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
            EXPECT_NE(graphicsQueue, nullptr);
            cgpu_free_queue(graphicsQueue);
        }
    }

    SUBCASE("GetSameQueue")
    {
        CGPUQueueId graphicsQueue;
        CGPUQueueId graphicsQueueSame;
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        if (gQueue > 0)
        {
            graphicsQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
            EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
            graphicsQueueSame = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
            EXPECT_EQ(graphicsQueue, graphicsQueueSame);
            cgpu_free_queue(graphicsQueue);
        }
    }

    SUBCASE("CreateCommands")
    {
        CGPUQueueId graphicsQueue;
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        if (gQueue > 0)
        {
            graphicsQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
            EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
            EXPECT_NE(graphicsQueue, nullptr);

            auto pool = cgpu_create_command_pool(graphicsQueue, nullptr);
            EXPECT_NE(pool, CGPU_NULLPTR);
            {
                SKR_DECLARE_ZERO(CGPUCommandBufferDescriptor, desc);
                desc.is_secondary = false;
                auto cmd = cgpu_create_command_buffer(pool, &desc);
                EXPECT_NE(cmd, CGPU_NULLPTR);
                cgpu_free_command_buffer(cmd);
            }
            cgpu_free_command_pool(pool);
            cgpu_free_queue(graphicsQueue);
        }
    }

    SUBCASE("TransferCmdReadback")
    {
        // Create Upload Buffer
        CGPUBufferId upload_buffer, index_buffer;
        {
            SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
            desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
            desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
            desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
            desc.element_stride = sizeof(uint16_t);
            desc.element_count = 3;
            desc.size = sizeof(uint16_t) * 3;
            desc.name = u8"UploadBuffer";
            upload_buffer = cgpu_create_buffer(device, &desc);
            EXPECT_NE(upload_buffer, CGPU_NULLPTR);
            EXPECT_NE(upload_buffer->info->cpu_mapped_address, CGPU_NULLPTR);
            uint16_t* indices = (uint16_t*)upload_buffer->info->cpu_mapped_address;
            indices[0] = 2;
            indices[1] = 3;
            indices[2] = 3;
        }
        {
            SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
            desc.flags = CGPU_BCF_NONE;
            desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
            desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
            desc.memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU;
            desc.element_stride = sizeof(uint16_t);
            desc.element_count = 3;
            desc.size = sizeof(uint16_t) * 3;
            desc.name = u8"ReadbackBuffer";
            index_buffer = cgpu_create_buffer(device, &desc);
            EXPECT_NE(index_buffer, CGPU_NULLPTR);
            EXPECT_EQ(index_buffer->info->cpu_mapped_address, CGPU_NULLPTR);
        }
        // Transfer
        CGPUQueueId transferQueue;
        auto tQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER);
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        if (tQueue > 0 || gQueue > 0)
        {
            if (tQueue > 0)
                transferQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_TRANSFER, 0);
            else
                transferQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

            EXPECT_NE(transferQueue, CGPU_NULLPTR);
            EXPECT_NE(transferQueue, nullptr);

            auto pool = cgpu_create_command_pool(transferQueue, nullptr);
            EXPECT_NE(pool, CGPU_NULLPTR);
            {
                SKR_DECLARE_ZERO(CGPUCommandBufferDescriptor, desc);
                desc.is_secondary = false;
                auto cmd = cgpu_create_command_buffer(pool, &desc);
                EXPECT_NE(cmd, CGPU_NULLPTR);
                {
                    cgpu_cmd_begin(cmd);
                    SKR_DECLARE_ZERO(CGPUBufferToBufferTransfer, cpy_desc);
                    cpy_desc.src = upload_buffer;
                    cpy_desc.src_offset = 0;
                    cpy_desc.dst = index_buffer;
                    cpy_desc.dst_offset = 0;
                    cpy_desc.size = sizeof(uint16_t) * 3;
                    cgpu_cmd_transfer_buffer_to_buffer(cmd, &cpy_desc);
                    cgpu_cmd_end(cmd);
                }
                CGPUQueueSubmitDescriptor submit_desc = {};
                submit_desc.cmds = &cmd;
                submit_desc.cmds_count = 1;
                cgpu_submit_queue(transferQueue, &submit_desc);
                cgpu_wait_queue_idle(transferQueue);
                {
                    SKR_DECLARE_ZERO(CGPUBufferRange, range);
                    range.offset = 0;
                    range.size = sizeof(uint16_t) * 3;
                    cgpu_map_buffer(index_buffer, &range);
                    uint16_t* read_indices = (uint16_t*)index_buffer->info->cpu_mapped_address;
                    EXPECT_EQ(read_indices[0], 2);
                    EXPECT_EQ(read_indices[1], 3);
                    EXPECT_EQ(read_indices[2], 3);
                    cgpu_unmap_buffer(index_buffer);
                }
                cgpu_free_command_buffer(cmd);
            }
            cgpu_free_command_pool(pool);
            cgpu_free_queue(transferQueue);
        }
        cgpu_free_buffer(upload_buffer);
        cgpu_free_buffer(index_buffer);
    }
}

#ifdef CGPU_USE_D3D12
TEST_CASE_METHOD(QueueOperations<CGPU_BACKEND_D3D12>, "QueueOperations-d3d12")
{
    test_all();
}
#endif

#ifdef CGPU_USE_VULKAN
TEST_CASE_METHOD(QueueOperations<CGPU_BACKEND_VULKAN>, "QueueOperations-vulkan")
{
    test_all();
}
#endif
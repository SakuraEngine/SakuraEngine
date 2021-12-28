#include "gtest/gtest.h"
#include "cgpu/api.h"

class QueueOperations : public ::testing::TestWithParam<ECGpuBackend>
{
protected:
    void SetUp() override
    {
        ECGpuBackend backend = GetParam();
        DECLARE_ZERO(CGpuInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = true;
        desc.enable_gpu_based_validation = true;
        desc.enable_set_name = true;
        instance = cgpu_create_instance(&desc);
        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGpuAdapterId> adapters;
        adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        for (auto a : adapters)
        {
            auto gQueue = cgpu_query_queue_count(a, QUEUE_TYPE_GRAPHICS);
            auto cQueue = cgpu_query_queue_count(a, QUEUE_TYPE_COMPUTE);
            auto tQueue = cgpu_query_queue_count(a, QUEUE_TYPE_TRANSFER);

            std::vector<CGpuQueueGroupDescriptor> queueGroup;
            if (gQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_GRAPHICS, 1 });
            if (cQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_COMPUTE, 1 });
            if (tQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_TRANSFER, 1 });
            DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
            descriptor.queueGroups = queueGroup.data();
            descriptor.queueGroupCount = (uint32_t)queueGroup.size();

            device = cgpu_create_device(a, &descriptor);
            adapter = a;
            EXPECT_NE(device, nullptr);
            EXPECT_NE(device, CGPU_NULLPTR);
            break;
        }
    }

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    CGpuInstanceId instance;
    CGpuAdapterId adapter;
    CGpuDeviceId device;
};

TEST_P(QueueOperations, GetGraphicsQueue)
{
    CGpuQueueId graphicsQueue;
    auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
    if (gQueue > 0)
    {
        graphicsQueue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
        EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
        EXPECT_NE(graphicsQueue, nullptr);
        cgpu_free_queue(graphicsQueue);
    }
}

TEST_P(QueueOperations, GetSameQueue)
{
    CGpuQueueId graphicsQueue;
    CGpuQueueId graphicsQueueSame;
    auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
    if (gQueue > 0)
    {
        graphicsQueue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
        EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
        graphicsQueueSame = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
        EXPECT_EQ(graphicsQueue, graphicsQueueSame);
        cgpu_free_queue(graphicsQueue);
    }
}

TEST_P(QueueOperations, CreateCommands)
{
    CGpuQueueId graphicsQueue;
    auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
    if (gQueue > 0)
    {
        graphicsQueue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
        EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
        EXPECT_NE(graphicsQueue, nullptr);

        auto pool = cgpu_create_command_pool(graphicsQueue, nullptr);
        EXPECT_NE(pool, CGPU_NULLPTR);
        {
            DECLARE_ZERO(CGpuCommandBufferDescriptor, desc);
            desc.is_secondary = false;
            auto cmd = cgpu_create_command_buffer(pool, &desc);
            EXPECT_NE(cmd, CGPU_NULLPTR);
            cgpu_free_command_buffer(cmd);
        }
        cgpu_free_command_pool(pool);
        cgpu_free_queue(graphicsQueue);
    }
}

TEST_P(QueueOperations, TransferCmdReadback)
{
    // Create Upload Buffer
    CGpuBufferId upload_buffer, index_buffer;
    {
        DECLARE_ZERO(CGpuBufferDescriptor, desc)
        desc.flags = BCF_PERSISTENT_MAP_BIT;
        desc.descriptors = RT_BUFFER;
        desc.memory_usage = MEM_USAGE_CPU_ONLY;
        desc.element_stride = sizeof(uint16_t);
        desc.elemet_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = "UploadBuffer";
        upload_buffer = cgpu_create_buffer(device, &desc);
        EXPECT_NE(upload_buffer, CGPU_NULLPTR);
        EXPECT_NE(upload_buffer->cpu_mapped_address, CGPU_NULLPTR);
        uint16_t* indices = (uint16_t*)upload_buffer->cpu_mapped_address;
        indices[0] = 2;
        indices[1] = 3;
        indices[2] = 3;
    }
    {
        DECLARE_ZERO(CGpuBufferDescriptor, desc)
        desc.flags = BCF_OWN_MEMORY_BIT;
        desc.descriptors = RT_NONE;
        desc.start_state = RESOURCE_STATE_COPY_DEST;
        desc.memory_usage = MEM_USAGE_GPU_TO_CPU;
        desc.element_stride = sizeof(uint16_t);
        desc.elemet_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = "ReadbackBuffer";
        index_buffer = cgpu_create_buffer(device, &desc);
        EXPECT_NE(index_buffer, CGPU_NULLPTR);
        EXPECT_EQ(index_buffer->cpu_mapped_address, CGPU_NULLPTR);
    }
    // Transfer
    CGpuQueueId transferQueue;
    auto tQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_TRANSFER);
    auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
    if (tQueue > 0 || gQueue > 0)
    {
        if (tQueue > 0)
            transferQueue = cgpu_get_queue(device, QUEUE_TYPE_TRANSFER, 0);
        else if (gQueue > 0)
            transferQueue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);

        EXPECT_NE(transferQueue, CGPU_NULLPTR);
        EXPECT_NE(transferQueue, nullptr);

        auto pool = cgpu_create_command_pool(transferQueue, nullptr);
        EXPECT_NE(pool, CGPU_NULLPTR);
        {
            DECLARE_ZERO(CGpuCommandBufferDescriptor, desc);
            desc.is_secondary = false;
            auto cmd = cgpu_create_command_buffer(pool, &desc);
            EXPECT_NE(cmd, CGPU_NULLPTR);
            {
                cgpu_cmd_begin(cmd);
                DECLARE_ZERO(CGpuBufferToBufferTransfer, cpy_desc);
                cpy_desc.src = upload_buffer;
                cpy_desc.src_offset = 0;
                cpy_desc.dst = index_buffer;
                cpy_desc.dst_offset = 0;
                cpy_desc.size = sizeof(uint16_t) * 3;
                cgpu_cmd_transfer_buffer_to_buffer(cmd, &cpy_desc);
                cgpu_cmd_end(cmd);
            }
            CGpuQueueSubmitDescriptor submit_desc = {};
            submit_desc.cmds = &cmd;
            submit_desc.cmds_count = 1;
            cgpu_submit_queue(transferQueue, &submit_desc);
            cgpu_wait_queue_idle(transferQueue);
            {
                DECLARE_ZERO(CGpuBufferRange, range);
                range.offset = 0;
                range.size = sizeof(uint16_t) * 3;
                cgpu_map_buffer(index_buffer, &range);
                uint16_t* read_indices = (uint16_t*)index_buffer->cpu_mapped_address;
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

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
    CGPU_BACKEND_VULKAN
#endif
#ifdef CGPU_USE_D3D12
    ,
    CGPU_BACKEND_D3D12
#endif
);

INSTANTIATE_TEST_SUITE_P(QueueOperations, QueueOperations, allPlatforms);
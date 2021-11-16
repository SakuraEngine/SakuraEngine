#define RUNTIME_DLL

#include "gtest/gtest.h"
#include "cgpu/api.h"

class QueueOperations : public::testing::TestWithParam<ECGPUBackEnd>
{
protected:
	void SetUp() override
	{
        ECGPUBackEnd backend = GetParam();
        CGpuInstanceDescriptor desc;
        desc.backend = backend;
        desc.enableDebugLayer = true;
        desc.enableGpuBasedValidation = true;
        instance = cgpu_create_instance(&desc);
        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGpuAdapterId> adapters; adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        for(auto a : adapters)
        {
            auto gQueue = cgpu_query_queue_count(a, ECGpuQueueType_Graphics); 
            auto cQueue = cgpu_query_queue_count(a, ECGpuQueueType_Compute); 
            auto tQueue = cgpu_query_queue_count(a, ECGpuQueueType_Transfer); 

            std::vector<CGpuQueueGroupDescriptor> queueGroup; 
            if(gQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Graphics, 1});
            if(cQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Compute, 1});
            if(tQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Transfer, 1});
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
    auto gQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Graphics); 
    if (gQueue > 0)
    {
        graphicsQueue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
        EXPECT_NE(graphicsQueue, nullptr);
        cgpu_free_queue(graphicsQueue);
    }
}

TEST_P(QueueOperations, CreateCommandEncoder)
{
    CGpuQueueId graphicsQueue;
    auto gQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Graphics); 
    if (gQueue > 0)
    {
        graphicsQueue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        EXPECT_NE(graphicsQueue, CGPU_NULLPTR);
        EXPECT_NE(graphicsQueue, nullptr);

        auto encoder = cgpu_create_command_encoder(graphicsQueue, nullptr);
        EXPECT_NE(encoder, CGPU_NULLPTR);
        EXPECT_NE(encoder, nullptr);

        cgpu_free_command_encoder(encoder);
        cgpu_free_queue(graphicsQueue);
    }
}

static const auto allPlatforms = testing::Values(
#ifndef TEST_WEBGPU    
    #ifdef CGPU_USE_VULKAN
        ECGPUBackEnd_VULKAN
    #endif
    #ifdef CGPU_USE_D3D12
        ,ECGPUBackEnd_D3D12
    #endif
#endif
);

INSTANTIATE_TEST_SUITE_P(QueueOperations, QueueOperations, allPlatforms);
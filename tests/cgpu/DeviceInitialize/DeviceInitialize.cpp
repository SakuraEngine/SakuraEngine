#define RUNTIME_DLL

#include "gtest/gtest.h"
#include "cgpu/api.h"

class CGpuTest : public::testing::TestWithParam<ECGPUBackEnd>
{
protected:
	void SetUp() override
	{

    }
};

CGpuInstanceId init_instance(ECGPUBackEnd backend, bool enableDebugLayer, bool enableGPUValidation)
{
    CGpuInstanceDescriptor desc;
    desc.backend = backend;
    desc.enableDebugLayer = enableDebugLayer;
    desc.enableGpuBasedValidation = enableGPUValidation;
    CGpuInstanceId instance = cgpu_create_instance(&desc);
    return instance;
}

int enum_adapters(CGpuInstanceId instance)
{
    // enum adapters
    size_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters; adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for(auto adapter : adapters)
    {
        auto prop = cgpu_query_adapter_detail(adapter);
        std::cout << "device id: " << prop.deviceId << "  vendor id: " << prop.vendorId << "\n";
        std::cout << "    name: " << prop.name << "\n";
    }
    //cgpu_free_instance(instance);
    return adapters_count;
}

void test_create_device(CGpuInstanceId instance, bool enableDebugLayer, bool enableGPUValidation)
{
    // enum adapters
    size_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters; adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for(auto adapter : adapters)
    {
        auto gQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Graphics); 
        auto cQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Compute); 
        auto tQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Transfer); 

        std::vector<CGpuQueueGroupDescriptor> queueGroup; 
        if(gQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Graphics, 1});
        if(cQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Compute, 1});
        if(tQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ECGpuQueueType_Transfer, 1});
        CGpuDeviceDescriptor descriptor = {};
        descriptor.queueGroups = queueGroup.data();
        descriptor.queueGroupCount = queueGroup.size();

        auto device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
        cgpu_free_device(device);
    }
}

TEST_P(CGpuTest, InstanceCreationDbgGpu)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, InstanceCreationDbg)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, InstanceCreation)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_P(CGpuTest, AdapterEnum)
{
    ECGPUBackEnd backend = GetParam();
    auto instance = init_instance(backend, true, true);
    EXPECT_GT(enum_adapters(instance), 0);
    cgpu_free_instance(instance);
}

TEST_P(CGpuTest, CreateDevice)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, CreateDeviceDbg)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, CreateDeviceDbgGpu)
{
    ECGPUBackEnd backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_P(CGpuTest, QueryQueueCount)
{
    ECGPUBackEnd backend = GetParam();
    auto instance = init_instance(backend, true, true);
    EXPECT_GT(enum_adapters(instance), 0);
    size_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters; adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for(auto adapter : adapters)
    {
        auto prop = cgpu_query_adapter_detail(adapter);
        auto gQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Graphics);
        auto cQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Compute);
        auto tQueue = cgpu_query_queue_count(adapter, ECGpuQueueType_Transfer);
        std::cout << prop.name << " of " << backend << "  "
            << "GraphicsQueue: " << gQueue << "  "
            << "ComputeQueue: " << cQueue << "  "
            << "TransferQueue: " << tQueue << std::endl;
    }
    cgpu_free_instance(instance);
}

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
    ECGPUBackEnd_VULKAN
#endif
#ifdef CGPU_USE_D3D12
    ECGPUBackEnd_D3D12
#endif
);

INSTANTIATE_TEST_SUITE_P(DeviceInitialization, CGpuTest, allPlatforms);

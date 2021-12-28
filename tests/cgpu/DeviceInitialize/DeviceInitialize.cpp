#include "gtest/gtest.h"
#include "cgpu/api.h"

class CGpuTest : public ::testing::TestWithParam<ECGpuBackend>
{
protected:
    void SetUp() override
    {
    }

    const char* GetBackendName()
    {
        ECGpuBackend backend = GetParam();
        switch (backend)
        {
            case ECGpuBackend::CGPU_BACKEND_D3D12:
                return "D3D12";
            case ECGpuBackend::CGPU_BACKEND_METAL:
                return "Metal";
            case ECGpuBackend::CGPU_BACKEND_VULKAN:
                return "Vulkan";
            case ECGpuBackend::CGPU_BACKEND_AGC:
                return "AGC";
            default:
                return "UNKNOWN";
        }
    }
};

CGpuInstanceId init_instance(ECGpuBackend backend, bool enable_debug_layer, bool enableGPUValidation)
{
    DECLARE_ZERO(CGpuInstanceDescriptor, desc)
    desc.backend = backend;
    desc.enable_debug_layer = enable_debug_layer;
    desc.enable_gpu_based_validation = enableGPUValidation;
    CGpuInstanceId instance = cgpu_create_instance(&desc);
    DECLARE_ZERO(CGpuInstanceFeatures, instance_features)
    cgpu_query_instance_features(instance, &instance_features);
    if (backend == ECGpuBackend::CGPU_BACKEND_VULKAN)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    else if (backend == ECGpuBackend::CGPU_BACKEND_D3D12)
    {
        EXPECT_FALSE(instance_features.specialization_constant);
    }
    else if (backend == ECGpuBackend::CGPU_BACKEND_METAL)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    return instance;
}

int enum_adapters(CGpuInstanceId instance)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGpuAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        const auto& VendorInfo = prop->vendor_preset;
        std::cout << " device id: " << VendorInfo.device_id
                  << " vendor id: " << VendorInfo.vendor_id << "\n"
                  << " name: " << VendorInfo.gpu_name << "\n"
                  << std::endl;
    }
    // cgpu_free_instance(instance);
    return adapters_count;
}

void test_create_device(CGpuInstanceId instance, bool enable_debug_layer, bool enableGPUValidation)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
        auto cQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_COMPUTE);
        auto tQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_TRANSFER);

        std::vector<CGpuQueueGroupDescriptor> queueGroup;
        if (gQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_GRAPHICS, 1 });
        if (cQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_COMPUTE, 1 });
        if (tQueue > 0) queueGroup.push_back(CGpuQueueGroupDescriptor{ QUEUE_TYPE_TRANSFER, 1 });
        DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
        descriptor.queueGroups = queueGroup.data();
        descriptor.queueGroupCount = (uint32_t)queueGroup.size();

        auto device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
        cgpu_free_device(device);
    }
}

TEST_P(CGpuTest, InstanceCreationDbgGpu)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, InstanceCreationDbg)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, InstanceCreation)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_P(CGpuTest, AdapterEnum)
{
    ECGpuBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    EXPECT_GT(enum_adapters(instance), 0);
    cgpu_free_instance(instance);
}

TEST_P(CGpuTest, CreateDeviceDbgGpu)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, CreateDevice)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(CGpuTest, CreateDeviceDbg)
{
    ECGpuBackend backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_P(CGpuTest, QueryQueueCount)
{
    ECGpuBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGpuAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        auto gQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_GRAPHICS);
        auto cQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_COMPUTE);
        auto tQueue = cgpu_query_queue_count(adapter, QUEUE_TYPE_TRANSFER);
        std::cout << prop->vendor_preset.gpu_name
                  << " of backend " << GetBackendName() << "  \n"
                  << "    GraphicsQueue: " << gQueue << "  \n"
                  << "    ComputeQueue: " << cQueue << "  \n"
                  << "    TransferQueue: " << tQueue << std::endl;
    }
    cgpu_free_instance(instance);
}

TEST_P(CGpuTest, QueryVendorInfo)
{
    ECGpuBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGpuAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGpuAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        std::cout << prop->vendor_preset.gpu_name << " Vendor Information (" << GetBackendName() << ")  \n"
                  << "    GPU Name: " << prop->vendor_preset.gpu_name << "\n"
                  << "    Is UMA: " << prop->is_uma << "\n"
                  << "    Device ID: " << prop->vendor_preset.device_id << "\n"
                  << "    Vendor ID: " << prop->vendor_preset.vendor_id << "\n"
                  << "    Driver Version: " << prop->vendor_preset.driver_version << "\n"
                  << std::endl;
    }
    cgpu_free_instance(instance);
}

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
    CGPU_BACKEND_VULKAN
#endif
#ifdef CGPU_USE_D3D12
    ,
    CGPU_BACKEND_D3D12
#endif
#ifdef CGPU_USE_METAL
    ,
    ECGpuBackend_METAL
#endif
);

INSTANTIATE_TEST_SUITE_P(DeviceInitialization, CGpuTest, allPlatforms);

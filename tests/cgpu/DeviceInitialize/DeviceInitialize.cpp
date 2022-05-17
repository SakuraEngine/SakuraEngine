#include "gtest/gtest.h"
#include "cgpu/api.h"

class Test : public ::testing::TestWithParam<EBackend>
{
protected:
    void SetUp() override
    {
    }

    const char* GetBackendName()
    {
        EBackend backend = GetParam();
        switch (backend)
        {
            case EBackend::CGPU_BACKEND_D3D12:
                return "D3D12";
            case EBackend::CGPU_BACKEND_METAL:
                return "Metal";
            case EBackend::CGPU_BACKEND_VULKAN:
                return "Vulkan";
            case EBackend::CGPU_BACKEND_AGC:
                return "AGC";
            default:
                return "UNKNOWN";
        }
    }
};

InstanceId init_instance(EBackend backend, bool enable_debug_layer, bool enableGPUValidation)
{
    DECLARE_ZERO(InstanceDescriptor, desc)
    desc.backend = backend;
    desc.enable_debug_layer = enable_debug_layer;
    desc.enable_gpu_based_validation = enableGPUValidation;
    InstanceId instance = cgpu_create_instance(&desc);
    DECLARE_ZERO(InstanceFeatures, instance_features)
    cgpu_query_instance_features(instance, &instance_features);
    if (backend == EBackend::CGPU_BACKEND_VULKAN)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    else if (backend == EBackend::CGPU_BACKEND_D3D12)
    {
        EXPECT_FALSE(instance_features.specialization_constant);
    }
    else if (backend == EBackend::CGPU_BACKEND_METAL)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    return instance;
}

int enum_adapters(InstanceId instance)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<AdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const AdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        const auto& VendorInfo = prop->vendor_preset;
        std::cout << " device id: " << VendorInfo.device_id
                  << " vendor id: " << VendorInfo.vendor_id << "\n"
                  << " name: " << VendorInfo.gpu_name << "\n"
                  << std::endl;
    }
    // cgpu_free_instance(instance);
    return adapters_count;
}

void test_create_device(InstanceId instance, bool enable_debug_layer, bool enableGPUValidation)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<AdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        auto cQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_COMPUTE);
        auto tQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER);

        std::vector<QueueGroupDescriptor> queueGroup;
        if (gQueue > 0) queueGroup.push_back(QueueGroupDescriptor{ CGPU_QUEUE_TYPE_GRAPHICS, 1 });
        if (cQueue > 0) queueGroup.push_back(QueueGroupDescriptor{ CGPU_QUEUE_TYPE_COMPUTE, 1 });
        if (tQueue > 0) queueGroup.push_back(QueueGroupDescriptor{ CGPU_QUEUE_TYPE_TRANSFER, 1 });
        DECLARE_ZERO(DeviceDescriptor, descriptor)
        descriptor.queueGroups = queueGroup.data();
        descriptor.queueGroupCount = (uint32_t)queueGroup.size();

        auto device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
        cgpu_free_device(device);
    }
}

TEST_P(Test, InstanceCreationDbgGpu)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(Test, InstanceCreationDbg)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}
TEST_P(Test, InstanceCreation)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_P(Test, AdapterEnum)
{
    EBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    EXPECT_GT(enum_adapters(instance), 0);
    cgpu_free_instance(instance);
}

TEST_P(Test, CreateDeviceDbgGpu)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(Test, CreateDevice)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}
TEST_P(Test, CreateDeviceDbg)
{
    EBackend backend = GetParam();
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_P(Test, QueryQueueCount)
{
    EBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<AdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const AdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        auto cQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_COMPUTE);
        auto tQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER);
        std::cout << prop->vendor_preset.gpu_name
                  << " of backend " << GetBackendName() << "  \n"
                  << "    GraphicsQueue: " << gQueue << "  \n"
                  << "    ComputeQueue: " << cQueue << "  \n"
                  << "    TransferQueue: " << tQueue << std::endl;
    }
    cgpu_free_instance(instance);
}

TEST_P(Test, QueryVendorInfo)
{
    EBackend backend = GetParam();
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<AdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const AdapterDetail* prop = cgpu_query_adapter_detail(adapter);
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
CGPU_BACKEND_METAL
#endif
);

INSTANTIATE_TEST_SUITE_P(DeviceInitialization, Test, allPlatforms);

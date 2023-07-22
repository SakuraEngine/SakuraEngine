#include "cgpu/api.h"
#include "SkrTestFramework/framework.hpp"
#include "SkrTestFramework/generators.hpp"
#include <catch2/generators/catch_generators.hpp>
#include <iostream>

class DeviceInitializeTest
{
protected:
    DeviceInitializeTest()
    {
        backend = GENERATE(CGPUBackendGenerator::Create());
    }

    const char* GetBackendName()
    {
        switch (backend)
        {
            case ECGPUBackend::CGPU_BACKEND_D3D12:
                return "D3D12";
            case ECGPUBackend::CGPU_BACKEND_METAL:
                return "Metal";
            case ECGPUBackend::CGPU_BACKEND_VULKAN:
                return "Vulkan";
            case ECGPUBackend::CGPU_BACKEND_AGC:
                return "AGC";
            default:
                return "UNKNOWN";
        }
    }
    ECGPUBackend backend;
};

CGPUInstanceId init_instance(ECGPUBackend backend, bool enable_debug_layer, bool enableGPUValidation)
{
    DECLARE_ZERO(CGPUInstanceDescriptor, desc)
    desc.backend = backend;
    desc.enable_debug_layer = enable_debug_layer;
    desc.enable_gpu_based_validation = enableGPUValidation;
    CGPUInstanceId instance = cgpu_create_instance(&desc);
    DECLARE_ZERO(CGPUInstanceFeatures, instance_features)
    cgpu_query_instance_features(instance, &instance_features);
    if (backend == ECGPUBackend::CGPU_BACKEND_VULKAN)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    else if (backend == ECGPUBackend::CGPU_BACKEND_D3D12)
    {
        EXPECT_FALSE(instance_features.specialization_constant);
    }
    else if (backend == ECGPUBackend::CGPU_BACKEND_METAL)
    {
        EXPECT_TRUE(instance_features.specialization_constant);
    }
    return instance;
}

int enum_adapters(CGPUInstanceId instance)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGPUAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGPUAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
        const auto& VendorInfo = prop->vendor_preset;
        std::cout << " device id: " << VendorInfo.device_id
                  << " vendor id: " << VendorInfo.vendor_id << "\n"
                  << " name: " << VendorInfo.gpu_name << "\n"
                  << std::endl;
    }
    // cgpu_free_instance(instance);
    return adapters_count;
}

void test_create_device(CGPUInstanceId instance, bool enable_debug_layer, bool enableGPUValidation)
{
    // enum adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGPUAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        auto gQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_GRAPHICS);
        auto cQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_COMPUTE);
        auto tQueue = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER);

        std::vector<CGPUQueueGroupDescriptor> queueGroup;
        if (gQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_GRAPHICS, 1 });
        if (cQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_COMPUTE, 1 });
        if (tQueue > 0) queueGroup.push_back(CGPUQueueGroupDescriptor{ CGPU_QUEUE_TYPE_TRANSFER, 1 });
        DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
        descriptor.queue_groups = queueGroup.data();
        descriptor.queue_group_count = (uint32_t)queueGroup.size();

        auto device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
        cgpu_free_device(device);
    }
}

TEST_CASE_METHOD(DeviceInitializeTest, "InstanceCreationDbgGpu")
{
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "InstanceCreationDbg")
{
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "InstanceCreation")
{
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "AdapterEnum")
{
    auto instance = init_instance(backend, true, true);
    REQUIRE(enum_adapters(instance) > 0);
    cgpu_free_instance(instance);
}

TEST_CASE_METHOD(DeviceInitializeTest, "CreateDeviceDbgGpu")
{
    auto inst = init_instance(backend, true, true);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "CreateDevice")
{
    auto inst = init_instance(backend, false, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "CreateDeviceDbg")
{
    auto inst = init_instance(backend, true, false);
    EXPECT_NE(inst, CGPU_NULLPTR);
    test_create_device(inst, false, false);
    cgpu_free_instance(inst);
}

TEST_CASE_METHOD(DeviceInitializeTest, "QueryQueueCount")
{
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGPUAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGPUAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
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

TEST_CASE_METHOD(DeviceInitializeTest, "QueryVendorInfo")
{
    auto instance = init_instance(backend, true, true);
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    std::vector<CGPUAdapterId> adapters;
    adapters.resize(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    for (auto adapter : adapters)
    {
        const CGPUAdapterDetail* prop = cgpu_query_adapter_detail(adapter);
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
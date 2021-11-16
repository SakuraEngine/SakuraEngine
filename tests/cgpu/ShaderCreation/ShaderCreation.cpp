#define RUNTIME_DLL
#include "gtest/gtest.h"
#include <EASTL/string.h>
#include "cgpu/api.h"
#include "spirv.h"

class ShaderCreation : public::testing::TestWithParam<ECGPUBackEnd>
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
        adapter = adapters[0];

        CGpuQueueGroupDescriptor G = {ECGpuQueueType_Graphics, 1};
        CGpuDeviceDescriptor descriptor = {};
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
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

TEST_P(ShaderCreation, CreateModules)
{
    ECGPUBackEnd backend = GetParam();
    CGpuShaderLibraryDescriptor vdesc = {};
    vdesc.code = (const uint32_t*)triangle_vert_spirv;
    vdesc.code_size = sizeof(triangle_vert_spirv);
    vdesc.name = "VertexShaderLibrary";
    vdesc.stage = ECGpuShaderStage::SS_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);
    
    CGpuShaderLibraryDescriptor fdesc = {};
    fdesc.code = (const uint32_t*)triangle_frag_spirv;
    fdesc.code_size = sizeof(triangle_frag_spirv);
    fdesc.name = "FragmentShaderLibrary";
    fdesc.stage = ECGpuShaderStage::SS_FRAG;
    auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);

    cgpu_free_shader_module(vertex_shader);
    cgpu_free_shader_module(fragment_shader);
}

static const auto allPlatforms = testing::Values(
#ifndef TEST_WEBGPU    
    #ifdef CGPU_USE_VULKAN
        ECGPUBackEnd_VULKAN
    #endif
    #ifdef CGPU_USE_D3D12
        //,ECGPUBackEnd_D3D12
    #endif
#endif
);

INSTANTIATE_TEST_SUITE_P(ShaderCreation, ShaderCreation, allPlatforms);
#define RUNTIME_DLL
#include "gtest/gtest.h"
#include <EASTL/string.h>
#include <fstream>
#include "cgpu/api.h"

eastl::string VertexStage = "vert";
eastl::string FragmentStage = "frag";
class ShaderCreation : public::testing::TestWithParam<ECGPUBackEnd>
{
protected:
    eastl::string GetShaderPath(const eastl::string& stage)
    {
        ECGPUBackEnd backend = GetParam();
        eastl::string BaseDir = "Shaders/basic";
        switch (backend) {
        case ECGPUBackEnd::ECGPUBackEnd_VULKAN:
            return BaseDir.append(".").append(stage).append(".spirv");
        default:
            break;
        }
        return BaseDir;
    }

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

        const auto vs_name = GetShaderPath(VertexStage);
        FILE *vsf = fopen(vs_name.c_str(),"r");
        fseek(vsf, 0, SEEK_END);
        long lSize = ftell(vsf);
        vs_bytes_size = (lSize / 4) + 1;
        vs_bytes = (uint32_t*)malloc(lSize);
        rewind(vsf); 
        fread(vs_bytes, sizeof(char), lSize, vsf);
        fclose(vsf);

        const auto fs_name = GetShaderPath(FragmentStage);
        FILE *fsf = fopen(fs_name.c_str(),"r");
        fseek(fsf, 0, SEEK_END);
        long pSize = ftell(fsf);
        fs_bytes_size = (pSize / 4) + 1;
        fs_bytes = (uint32_t*)malloc(pSize);
        rewind(fsf); 
        fread(fs_bytes, sizeof(char), pSize, fsf);
        fclose(fsf);
    }

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
        free(vs_bytes);
        free(fs_bytes);
    }
    
    CGpuInstanceId instance;
    CGpuAdapterId adapter;
    CGpuDeviceId device;
    size_t vs_bytes_size; uint32_t* vs_bytes;
    size_t fs_bytes_size; uint32_t* fs_bytes; 
};

TEST_P(ShaderCreation, CreateModules)
{
    CGpuShaderModuleDescriptor vdesc = {};
    vdesc.code = vs_bytes; vdesc.code_size = vs_bytes_size;
    vdesc.name = "VertexShaderModule";
    auto vertex_shader = cgpu_create_shader_module(device, &vdesc);
    
    CGpuShaderModuleDescriptor fdesc = {};
    fdesc.code = fs_bytes; fdesc.code_size = fs_bytes_size;
    fdesc.name = "FragmentShaderModule";
    auto fragment_shader = cgpu_create_shader_module(device, &fdesc);

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);
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

INSTANTIATE_TEST_SUITE_P(ShaderCreation, ShaderCreation, allPlatforms);
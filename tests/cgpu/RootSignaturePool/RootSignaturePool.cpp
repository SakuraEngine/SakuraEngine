#include "gtest/gtest.h"
#include "cgpu/api.h"
#include "utils/make_zeroed.hpp"

class RootSignaturePool : public ::testing::TestWithParam<ECGPUBackend>
{
protected:
    void SetUp() override
    {
        backend = GetParam();
        DECLARE_ZERO(CGPUInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = true;
        desc.enable_gpu_based_validation = true;
        desc.enable_set_name = true;
        instance = cgpu_create_instance(&desc);

        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGPUAdapterId> adapters;
        adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        adapter = adapters[0];

        CGPUQueueGroupDescriptor G = { CGPU_QUEUE_TYPE_GRAPHICS, 1 };
        DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
    }

    CGPUSwapChainId CreateSwapChainWithSurface(CGPUSurfaceId surface)
    {
        auto mainQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
        DECLARE_ZERO(CGPUSwapChainDescriptor, descriptor)
        descriptor.presentQueues = &mainQueue;
        descriptor.presentQueuesCount = 1;
        descriptor.surface = surface;
        descriptor.imageCount = 3;
        descriptor.format = CGPU_FORMAT_R8G8B8A8_UNORM;
        descriptor.enableVsync = true;

        auto swapchain = cgpu_create_swapchain(device, &descriptor);
        return swapchain;
    }

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
    ECGPUBackend backend;
};

inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(
const char* virtual_path, uint32_t** bytes, uint32_t* length,
ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat(shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat(shader_file, ".dxil");
            break;
        default:
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

CGPURootSignatureId create_root_sig_with_shaders(CGPUDeviceId device,
CGPURootSignaturePoolId pool, ECGPUBackend backend,
const char8_t* vs, const char8_t* ps)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes(vs, &vs_bytes, &vs_length, backend);
    read_shader_bytes(ps, &fs_bytes, &fs_length, backend);
    auto vs_desc = make_zeroed<CGPUShaderLibraryDescriptor>();
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    auto ps_desc = make_zeroed<CGPUShaderLibraryDescriptor>();
    ps_desc.name = "FragmentShaderLibrary";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    const char8_t* push_const_name = u8"push_constants";
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.pool = pool;
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_const_name;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    return root_sig;
}

// y
TEST_P(RootSignaturePool, PS00)
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = "RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    EXPECT_TRUE(rs1->pool_sig == rs0);
    cgpu_free_root_signature_pool(pool);
}

TEST_P(RootSignaturePool, RC)
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = "RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");

    EXPECT_TRUE(rs1->pool_sig == rs0);

    cgpu_free_root_signature(rs0);
    cgpu_free_root_signature(rs1);

    auto rs2 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");

    EXPECT_TRUE(rs2->pool_sig == nullptr);

    cgpu_free_root_signature_pool(pool);
}

// y
TEST_P(RootSignaturePool, PS01)
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = "RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader1");
    EXPECT_TRUE(rs1->pool_sig == rs0);
    cgpu_free_root_signature_pool(pool);
}

// n
TEST_P(RootSignaturePool, PS02)
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = "RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader2");
    EXPECT_TRUE(rs1->pool_sig != rs0);
    EXPECT_TRUE(rs1->pool_sig == nullptr);
    cgpu_free_root_signature_pool(pool);
}

// n
TEST_P(RootSignaturePool, PS03)
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = "RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader3");
    EXPECT_TRUE(rs1->pool_sig != rs0);
    EXPECT_TRUE(rs1->pool_sig == nullptr);
    cgpu_free_root_signature_pool(pool);
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

INSTANTIATE_TEST_SUITE_P(RootSignaturePool, RootSignaturePool, allPlatforms);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}
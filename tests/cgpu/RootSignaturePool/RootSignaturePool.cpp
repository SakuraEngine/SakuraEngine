#include "SkrGraphics/api.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include <vector>

#include "SkrTestFramework/framework.hpp"

template <ECGPUBackend backend>
class RootSignaturePool
{
protected:
    void Initialize() SKR_NOEXCEPT
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
        adapter = adapters[0];

        CGPUQueueGroupDescriptor G = { CGPU_QUEUE_TYPE_GRAPHICS, 1 };
        SKR_DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
        descriptor.queue_groups = &G;
        descriptor.queue_group_count = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);
    }

    CGPUSwapChainId CreateSwapChainWithSurface(CGPUSurfaceId surface)
    {
        auto mainQueue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
        SKR_DECLARE_ZERO(CGPUSwapChainDescriptor, descriptor)
        descriptor.present_queues = &mainQueue;
        descriptor.present_queues_count = 1;
        descriptor.surface = surface;
        descriptor.image_count = 3;
        descriptor.format = CGPU_FORMAT_R8G8B8A8_UNORM;
        descriptor.enable_vsync = true;

        auto swapchain = cgpu_create_swapchain(device, &descriptor);
        return swapchain;
    }

    RootSignaturePool() SKR_NOEXCEPT
    {
        Initialize();
    }

    ~RootSignaturePool() 
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    void test_all();

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
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
    const char8_t* virtual_path, uint32_t** bytes, uint32_t* length,
    ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, (const char*)virtual_path);
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
    vs_desc.name = u8"VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    auto ps_desc = make_zeroed<CGPUShaderLibraryDescriptor>();
    ps_desc.name = u8"FragmentShaderLibrary";
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = u8"main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = u8"main";
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

template <ECGPUBackend backend>
void RootSignaturePool<backend>::test_all()
{
// y
SUBCASE("PS00")
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = u8"RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
        u8"cgpu-rspool-test/vertex_shader",
        u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
        u8"cgpu-rspool-test/vertex_shader",
        u8"cgpu-rspool-test/fragment_shader0");
    REQUIRE(rs1->pool_sig == rs0);
    cgpu_free_root_signature_pool(pool);
}

SUBCASE("RC")
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = u8"RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
        u8"cgpu-rspool-test/vertex_shader",
        u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
        u8"cgpu-rspool-test/vertex_shader",
        u8"cgpu-rspool-test/fragment_shader0");

    REQUIRE(rs1->pool_sig == rs0);

    cgpu_free_root_signature(rs0);
    cgpu_free_root_signature(rs1);

    auto rs2 = create_root_sig_with_shaders(device, pool, backend,
        u8"cgpu-rspool-test/vertex_shader",
        u8"cgpu-rspool-test/fragment_shader0");

    REQUIRE(rs2->pool_sig == nullptr);

    cgpu_free_root_signature_pool(pool);
}

// y
SUBCASE("PS01")
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = u8"RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader1");
    REQUIRE(rs1->pool_sig == rs0);
    cgpu_free_root_signature_pool(pool);
}

// n
SUBCASE("PS02")
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = u8"RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader2");
    REQUIRE(rs1->pool_sig != rs0);
    REQUIRE(rs1->pool_sig == nullptr);
    cgpu_free_root_signature_pool(pool);
}

// n
SUBCASE("PS03")
{
    CGPURootSignaturePoolDescriptor pool_desc = {};
    pool_desc.name = u8"RSPool";
    auto pool = cgpu_create_root_signature_pool(device, &pool_desc);
    auto rs0 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader0");
    auto rs1 = create_root_sig_with_shaders(device, pool, backend,
    u8"cgpu-rspool-test/vertex_shader",
    u8"cgpu-rspool-test/fragment_shader3");
    REQUIRE(rs1->pool_sig != rs0);
    REQUIRE(rs1->pool_sig == nullptr);
    cgpu_free_root_signature_pool(pool);
}
}

#ifdef CGPU_USE_D3D12
TEST_CASE_METHOD(RootSignaturePool<CGPU_BACKEND_D3D12>, "RootSignaturePool-d3d12")
{
    test_all();
}
#endif

#ifdef CGPU_USE_VULKAN
TEST_CASE_METHOD(RootSignaturePool<CGPU_BACKEND_VULKAN>, "RootSignaturePool-vulkan")
{
    test_all();
}
#endif
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_mesh.h"
#include "cgpu/api.h"
#include "module/module_manager.hpp"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "SkrImGui/skr_imgui.h"
#include "platform/guid.hpp"
#include <string.h>
#ifdef _WIN32
#include "cgpu/extensions/cgpu_d3d12_exts.h"
#endif

IMPLEMENT_DYNAMIC_MODULE(SkrRendererModule, SkrRenderer);

namespace 
{
using namespace skr::guid::literals;
const auto kGLTFVertexLayoutWithoutTangentId = "1b357a40-83ff-471c-8903-23e99d95b273"_guid;
const auto kGLTFVertexLayoutWithTangentId = "1b11e007-7cc2-4941-bc91-82d992c4b489"_guid;
const auto kGLTFVertexLayoutWithJointId = "C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid;
}

void SkrRendererModule::on_load(int argc, char** argv)
{
    SKR_LOG_TRACE("skr renderer loaded!");
#ifdef _WIN32
    cgpu_d3d12_enable_DRED();
#endif
    render_device = skr::RendererDevice::Create();
    // initailize render device
    auto builder = make_zeroed<skr::RendererDevice::Builder>();
    builder.enable_debug_layer = false;
    builder.enable_gpu_based_validation = false;
    builder.enable_set_name = true;
    builder.aux_thread_count = 1;
    for (auto i = 0; i < argc; i++)
    {
        if (::strcmp(argv[i], "--vulkan") == 0)
        {
            builder.backend = CGPU_BACKEND_VULKAN;
        }
        else if (::strcmp(argv[i], "--d3d12") == 0)
        {
            builder.backend = CGPU_BACKEND_D3D12;
        }
        else
        {
#ifdef _WIN32
            builder.backend = CGPU_BACKEND_D3D12;
#else
            builder.backend = CGPU_BACKEND_VULKAN;
#endif
        }
        builder.enable_debug_layer |= (0 == ::strcmp(argv[i], "--debug_layer"));
        builder.enable_gpu_based_validation |= (0 == ::strcmp(argv[i], "--gpu_based_validation"));
        builder.enable_set_name |= (0 == ::strcmp(argv[i], "--gpu_obj_name"));
    }
    render_device->initialize(builder);

    // register vertex layout
    {
        CGPUVertexLayout vertex_layout = {};
        vertex_layout.attributes[0] = { u8"POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[1] = { u8"TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[2] = { u8"TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 2, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[3] = { u8"NORMAL", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 3, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[4] = { u8"TANGENT", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 4, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[5] = { u8"JOINTS", 1, CGPU_FORMAT_R32G32B32A32_UINT, 5, 0, sizeof(uint32_t) * 4, CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attributes[6] = { u8"WEIGHTS", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 6, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
        vertex_layout.attribute_count = 7;
        skr_mesh_resource_register_vertex_layout(::kGLTFVertexLayoutWithJointId, "SkinnedMesh", &vertex_layout);
        vertex_layout.attribute_count = 4;
        skr_mesh_resource_register_vertex_layout(::kGLTFVertexLayoutWithoutTangentId, "StaticMeshWithoutTangent", &vertex_layout);
        vertex_layout.attribute_count = 5;
        skr_mesh_resource_register_vertex_layout(::kGLTFVertexLayoutWithTangentId, "StaticMeshWithTangent", &vertex_layout);
    }
}

void SkrRendererModule::on_unload()
{
    SKR_LOG_TRACE("skr renderer unloaded!");

    render_device->finalize();
    skr::RendererDevice::Free(render_device);
}

SkrRendererModule* SkrRendererModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SkrRendererModule*>(mm->get_module("SkrRenderer"));
    return rm;
}

SRenderDeviceId SkrRendererModule::get_render_device()
{
    return render_device;
}

SRenderDeviceId skr_get_default_render_device()
{
    return SkrRendererModule::Get()->get_render_device();
}

CGPUSwapChainId skr_render_device_register_window(SRenderDeviceId device, SWindowHandle window)
{
    return device->register_window(window);
}

CGPUSwapChainId skr_render_device_recreate_window_swapchain(SRenderDeviceId device, SWindowHandle window)
{
    return device->recreate_window_swapchain(window);
}

ECGPUFormat skr_render_device_get_swapchain_format(SRenderDeviceId device)
{
    return device->get_swapchain_format();
}

CGPUSamplerId skr_render_device_get_linear_sampler(SRenderDeviceId device)
{
    return device->get_linear_sampler();
}

CGPURootSignaturePoolId skr_render_device_get_root_signature_pool(SRenderDeviceId device)
{
    return device->get_root_signature_pool();
}

CGPUQueueId skr_render_device_get_gfx_queue(SRenderDeviceId device)
{
    return device->get_gfx_queue();
}

CGPUQueueId skr_render_device_get_cpy_queue(SRenderDeviceId device)
{
    return device->get_cpy_queue();
}

CGPUQueueId skr_render_device_get_nth_cpy_queue(SRenderDeviceId device, uint32_t n)
{
    return device->get_cpy_queue(n);
}

CGPUDeviceId skr_render_device_get_cgpu_device(SRenderDeviceId device)
{
    return device->get_cgpu_device();
}

skr_io_vram_service_t* skr_render_device_get_vram_service(SRenderDeviceId device)
{
    return device->get_vram_service();
}

CGPUDStorageQueueId skr_render_device_get_file_dstorage_queue(SRenderDeviceId device)
{
    return device->get_file_dstorage_queue();
}
CGPUDStorageQueueId skr_render_device_get_memory_dstorage_queue(SRenderDeviceId device)
{
    return device->get_memory_dstorage_queue();
}

#ifdef _WIN32
skr_win_dstorage_decompress_service_id skr_render_device_get_win_dstorage_decompress_service(SRenderDeviceId device)
{
    return device->get_win_dstorage_decompress_service();
}
#endif
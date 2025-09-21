#pragma once
#include "SkrBase/math/gen/gen_math_c_decl.hpp"
#include "SkrGraphics/api.h"
#include "SkrCore/platform/vfs.h"
#include "SkrRenderer/render_device.h"
#include "SkrContainersDef/string.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "SkrBase/math.h"
#include <imgui.h>

namespace utils
{

inline uint32_t* read_shader_bytes(skr::RenderDevice* render_device, skr_vfs_t* resource_vfs, const char8_t* name, uint32_t* out_length)
{
    const auto cgpu_device = render_device->get_cgpu_device();
    const auto backend = cgpu_device->adapter->instance->backend;
    skr::String shader_name = name;
    shader_name.append(backend == ::CGPU_BACKEND_D3D12 ? u8".dxil" : u8".spv");
    auto shader_file = skr_vfs_fopen(resource_vfs, shader_name.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const uint32_t shader_length = (uint32_t)skr_vfs_fsize(shader_file);
    auto shader_bytes = (uint32_t*)sakura_malloc(shader_length);
    skr_vfs_fread(shader_file, shader_bytes, 0, shader_length);
    skr_vfs_fclose(shader_file);
    if (out_length) *out_length = shader_length;
    return shader_bytes;
}

inline CGPUShaderLibraryId create_shader_library(skr::RenderDevice* render_device, skr_vfs_t* resource_vfs, const char8_t* name, ECGPUShaderStage stage)
{
    const auto cgpu_device = render_device->get_cgpu_device();
    uint32_t shader_length = 0;
    uint32_t* shader_bytes = read_shader_bytes(render_device, resource_vfs, name, &shader_length);
    CGPUShaderLibraryDescriptor shader_desc = {};
    shader_desc.name = name;
    shader_desc.code = shader_bytes;
    shader_desc.code_size = shader_length;
    CGPUShaderLibraryId shader = cgpu_create_shader_library(cgpu_device, &shader_desc);
    sakura_free(shader_bytes);
    return shader;
}

struct Camera
{
    skr::float3 pos = { 5.0f, 10.0f, -10.0f }; // camera position
    skr::float3 front = skr::float3::forward();    // camera front vector
    skr::float3 up = skr::float3::up();            // camera up vector
    skr::float3 right = skr::float3::right();      // camera right vector

    float fov = 3.1415926f / 2.f; // fov_x
    float aspect = 1.0;           // aspect ratio
    float near_plane = 0.1;       // near plane distance
    float far_plane = 1000.0;     // far plane distance
};

class SCENE_RENDERER_API CameraController
{
    utils::Camera* camera = nullptr;

public:
    void set_camera(utils::Camera* cam) { camera = cam; }
    void imgui_camera_info_frame();
    void imgui_control_frame();
};

} // namespace utils

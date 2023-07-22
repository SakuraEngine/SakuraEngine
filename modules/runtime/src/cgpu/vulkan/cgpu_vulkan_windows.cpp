#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "../common/common_utils.h"
#include "vulkan_utils.h"
#ifdef CGPU_THREAD_SAFETY
    #include "SkrRT/platform/thread.h"
#endif

CGPU_EXTERN_C uint64_t cgpu_export_shared_texture_handle_vulkan_win32(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc)
{
    CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)desc->texture;
    const CGPUTextureInfo* info = T->super.info;

    // vulkan shared handles are specified at allocation time
    uint64_t shared_handle = info->unique_id;

    cgpu_trace(u8"Vulkan Win32 Exported shared texture %p handle %llu size %dx%dx%d", 
        T, shared_handle, info->width, info->height, info->depth);

#ifdef _DEBUG
    auto pid = (uint64_t)GetCurrentProcessId();
    cgpu_assert(pid == (shared_handle >> 32));
#endif

    return info->unique_id;
}

CGPU_EXTERN_C CGPUTextureId cgpu_import_shared_texture_handle_vulkan_win32(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc)
{
    CGPUTextureDescriptor tex_desc = {};
    tex_desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
    tex_desc.flags = CGPU_INNER_TCF_IMPORT_SHARED_HANDLE;
    tex_desc.width = desc->width;
    tex_desc.height = desc->height;
    tex_desc.depth = desc->depth;
    tex_desc.format = desc->format;
    tex_desc.mip_levels = desc->mip_levels;
    tex_desc.array_size = 1;
    tex_desc.native_handle = desc;
    tex_desc.is_restrict_dedicated = true;

    cgpu_trace(u8"Vulkan Win32 Imported shared texture handle %llu %dx%dx%d backend: %d", 
        desc->shared_handle, desc->width, desc->height, desc->depth, desc->backend);

    return cgpu_create_texture(device, &tex_desc);
}
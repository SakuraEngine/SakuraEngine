#include "SkrGuiRenderer/resource/skr_updatable_image.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_image_task.hpp"
#include "SkrRT/misc/make_zeroed.hpp"

namespace skr::gui
{
SkrUpdatableImage::SkrUpdatableImage(SkrRenderDevice* render_device)
    : _render_device(render_device)
{
}
SkrUpdatableImage::~SkrUpdatableImage()
{
    if (_cgpu_texture)
    {
        cgpu_free_texture(_cgpu_texture);
    }
}

EResourceState SkrUpdatableImage::state() const SKR_NOEXCEPT
{
    return _state;
}
void SkrUpdatableImage::request()
{
    // do noting
}
void SkrUpdatableImage::cancel_request()
{
    // do noting
}
void SkrUpdatableImage::destroy()
{
    if (_cgpu_texture)
    {
        cgpu_wait_queue_idle(_render_device->cgpu_queue());
        cgpu_free_texture(_cgpu_texture);
        cgpu_free_texture_view(_texture_view);
        cgpux_free_bind_table(_bind_table);
        _cgpu_texture = nullptr;
    }
    _state = EResourceState::Destroyed;
    SkrDelete(this);
}
Sizei SkrUpdatableImage::size() const SKR_NOEXCEPT
{
    return _desc.size;
}
Rectf SkrUpdatableImage::uv_rect() const SKR_NOEXCEPT
{
    return Rectf{ 0, 0, 1, 1 };
}
EdgeInsetsf SkrUpdatableImage::nine_inset() const SKR_NOEXCEPT
{
    return { 0, 0, 0, 0 };
}
void SkrUpdatableImage::update(const UpdatableImageDesc& desc)
{
    if (_cgpu_texture && (desc.size != _desc.size || _desc.format != desc.format))
    {
        cgpu_wait_queue_idle(_render_device->cgpu_queue());
        cgpu_free_texture(_cgpu_texture);
        cgpu_free_texture_view(_texture_view);
        cgpux_free_bind_table(_bind_table);
        _cgpu_texture = nullptr;
    }

    // prepare data
    auto           queue              = _render_device->cgpu_queue();
    auto           device             = _render_device->cgpu_device();
    auto           format             = pixel_format_to_cgpu_format(desc.format);
    auto           root_signature     = _render_device->get_pipeline(ESkrPipelineFlag_Textured)->root_signature;
    const char8_t* color_texture_name = u8"color_texture";

    // create texture
    CGPUTextureDescriptor tex_desc = {};
    tex_desc.name                  = color_texture_name;
    tex_desc.width                 = static_cast<uint32_t>(desc.size.width);
    tex_desc.height                = static_cast<uint32_t>(desc.size.height);
    tex_desc.depth                 = 1;
    tex_desc.descriptors           = CGPU_RESOURCE_TYPE_TEXTURE;
    tex_desc.array_size            = 1;
    tex_desc.flags                 = CGPU_TCF_NONE;
    tex_desc.mip_levels            = 1;
    tex_desc.format                = format;
    tex_desc.start_state           = CGPU_RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue           = queue;
    _cgpu_texture                  = cgpu_create_texture(queue->device, &tex_desc);

    // upload data
    size_t                      upload_size        = desc.data.size();
    CGPUCommandPoolDescriptor   cmd_pool_desc      = {};
    CGPUCommandBufferDescriptor cmd_desc           = {};
    CGPUBufferDescriptor        upload_buffer_desc = {};
    upload_buffer_desc.name                        = u8"updatable_image_upload_buffer";
    upload_buffer_desc.flags                       = CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors                 = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage                = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size                        = upload_size;
    CGPUBufferId tex_upload_buffer                 = cgpu_create_buffer(queue->device, &upload_buffer_desc);
    {
        memcpy(tex_upload_buffer->info->cpu_mapped_address, desc.data.data(), upload_size);
    }
    auto cpy_cmd_pool = cgpu_create_command_pool(queue, &cmd_pool_desc);
    auto cpy_cmd      = cgpu_create_command_buffer(cpy_cmd_pool, &cmd_desc);
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToTextureTransfer b2t      = {};
    b2t.src                              = tex_upload_buffer;
    b2t.src_offset                       = 0;
    b2t.dst                              = _cgpu_texture;
    b2t.dst_subresource.mip_level        = 0;
    b2t.dst_subresource.base_array_layer = 0;
    b2t.dst_subresource.layer_count      = 1;
    cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);
    CGPUTextureBarrier srv_barrier              = {};
    srv_barrier.texture                         = _cgpu_texture;
    srv_barrier.src_state                       = CGPU_RESOURCE_STATE_COPY_DEST;
    srv_barrier.dst_state                       = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
    CGPUResourceBarrierDescriptor barrier_desc1 = {};
    barrier_desc1.texture_barriers              = &srv_barrier;
    barrier_desc1.texture_barriers_count        = 1;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc1);
    cgpu_cmd_end(cpy_cmd);
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds                      = &cpy_cmd;
    cpy_submit.cmds_count                = 1;
    cgpu_submit_queue(queue, &cpy_submit);
    cgpu_wait_queue_idle(queue);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cpy_cmd_pool);
    cgpu_free_buffer(tex_upload_buffer);

    // create texture view
    CGPUTextureViewDescriptor view_desc = {};
    view_desc.texture                   = _cgpu_texture;
    view_desc.format                    = format;
    view_desc.array_layer_count         = 1;
    view_desc.base_array_layer          = 0;
    view_desc.mip_level_count           = 1;
    view_desc.base_mip_level            = 0;
    view_desc.aspects                   = CGPU_TVA_COLOR;
    view_desc.dims                      = CGPU_TEX_DIMENSION_2D;
    view_desc.usages                    = CGPU_TVU_SRV;
    _texture_view                       = cgpu_create_texture_view(device, &view_desc);

    // create bind table
    CGPUXBindTableDescriptor bind_table_desc = {};
    bind_table_desc.root_signature           = root_signature;
    bind_table_desc.names                    = &color_texture_name;
    bind_table_desc.names_count              = 1;
    _bind_table                              = cgpux_create_bind_table(device, &bind_table_desc);

    // update bind table
    auto data         = make_zeroed<CGPUDescriptorData>();
    data.name         = color_texture_name;
    data.count        = 1;
    data.binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
    data.textures     = &_texture_view;
    cgpux_bind_table_update(_bind_table, &data, 1);

    // record desc
    _desc = desc;
}
} // namespace skr::gui
#include "SkrGuiRenderer/renderer.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/defer.hpp"
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
#include "SkrImageCoder/skr_image_coder.h"
#endif

namespace skr {
namespace gdi {

EGDIResourceState SGDITexture_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomic32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

EGDITextureType SGDITexture_RenderGraph::get_type() const SKR_NOEXCEPT
{
    return EGDITextureType::Texture2D;
}

#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
ECGPUFormat cgpu_format_from_image_coder_format(EImageCoderFormat format,EImageCoderColorFormat cformat, uint32_t bit_depth) SKR_NOEXCEPT
{
    (void)bit_depth;
    if (format == IMAGE_CODER_FORMAT_JPEG || format == IMAGE_CODER_FORMAT_PNG)
    {
        switch (cformat)
        {
        case IMAGE_CODER_COLOR_FORMAT_RGBA: return CGPU_FORMAT_R8G8B8A8_UNORM;
        case IMAGE_CODER_COLOR_FORMAT_BGRA: return CGPU_FORMAT_B8G8R8A8_UNORM;
        case IMAGE_CODER_COLOR_FORMAT_Gray: return CGPU_FORMAT_R8_UNORM;

        case IMAGE_CODER_COLOR_FORMAT_GrayF:
        case IMAGE_CODER_COLOR_FORMAT_RGBAF: 
        case IMAGE_CODER_COLOR_FORMAT_BGRE: 
        default: 
            return CGPU_FORMAT_UNDEFINED;
        }
    }
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_FORMAT_UNDEFINED;
}

skr_blob_t image_coder_decode_image(uint8_t* bytes, uint64_t size, skr_vram_texture_io_t* vram_io)
{
    ZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)bytes, size);
    auto coder = skr_image_coder_create_image(format);
    if (skr_image_coder_set_encoded(coder, (const uint8_t*)bytes, size))
    {
        skr_blob_t output = {};
        SKR_DEFER({ skr_image_coder_free_image(coder); });
        const auto encoded_format = coder->get_color_format();
        const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
        if (vram_io)
        {
            const auto bit_depth = coder->get_bit_depth();
            vram_io->vtexture.depth = 1;
            vram_io->vtexture.height = coder->get_height();
            vram_io->vtexture.width = coder->get_width();
            vram_io->vtexture.format = cgpu_format_from_image_coder_format(format, raw_format, bit_depth);
            vram_io->vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;
        }
        coder->steal_raw_data(&output, raw_format, coder->get_bit_depth());
        return output;
    }
    return {};
}
#endif

void SGDITexture_RenderGraph::intializeBindTable() SKR_NOEXCEPT
{
    CGPUTextureViewDescriptor view_desc = {};
    view_desc.texture = texture;
    view_desc.format = (ECGPUFormat)texture->format;
    view_desc.array_layer_count = 1;
    view_desc.base_array_layer = 0;
    view_desc.mip_level_count = 1;
    view_desc.base_mip_level = 0;
    view_desc.aspects = CGPU_TVA_COLOR;
    view_desc.dims = CGPU_TEX_DIMENSION_2D;
    view_desc.usages = CGPU_TVU_SRV;
    texture_view = cgpu_create_texture_view(device, &view_desc);

    const char* color_texture_name = "color_texture";
    CGPUXBindTableDescriptor bind_table_desc = {};
    bind_table_desc.root_signature = root_signature;
    bind_table_desc.names = &color_texture_name;
    bind_table_desc.names_count = 1;
    bind_table = cgpux_create_bind_table(texture->device, &bind_table_desc);
    auto data = make_zeroed<CGPUDescriptorData>();
    data.name = color_texture_name;
    data.count = 1;
    data.binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
    data.textures = &texture_view;
    cgpux_bind_table_update(bind_table, &data, 1);
}

SGDITextureId SGDIRenderer_RenderGraph::create_texture(const SGDITextureDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const SGDITextureDescriptor_RenderGraph*>(desc->usr_data);
    auto texture = SkrNew<SGDITexture_RenderGraph>();
    texture->uri = desc->u8Uri;
    texture->aux_service = aux_service;
    texture->vram_service = vram_service;
    texture->device = device;
    texture->transfer_queue = transfer_queue;
    texture->root_signature = texture_pipeline->root_signature;
    const bool useImageCoder = desc2->useImageCoder;
    if (useImageCoder)
    {
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
        // 1. ram_service -> aux_decode -> vram_service
        auto ram_texture_io = make_zeroed<skr_ram_io_t>();
        ram_texture_io.path = texture->uri.c_str();
        ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
            skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Loading));

            auto aux_task = make_zeroed<skr_service_task_t>();
            aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                ZoneScopedN("CreateGUITexture(AuxService)");
                auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
                skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Initializing));

                auto vram_io_info = make_zeroed<skr_vram_texture_io_t>();
                texture->raw_data = image_coder_decode_image(texture->ram_destination.bytes, texture->ram_destination.size, &vram_io_info);
                vram_io_info.src_memory.bytes = texture->raw_data.bytes;
                vram_io_info.src_memory.size = texture->raw_data.size;
                vram_io_info.device = texture->device;
                vram_io_info.transfer_queue = texture->transfer_queue;
                vram_io_info.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
                {
                    auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
                    texture->texture = texture->vram_destination.texture;
                    texture->intializeBindTable();

                    skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Okay));
                    sakura_free(texture->raw_data.bytes);
                    texture->raw_data = {};
                };
                vram_io_info.callback_datas[SKR_ASYNC_IO_STATUS_OK] = texture;
                texture->vram_service->request(&vram_io_info, &texture->vram_request, &texture->vram_destination);
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = texture;
            texture->aux_service->request(&aux_task, &texture->aux_request);
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = texture;

        ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_ENQUEUED] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
            skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Loading));
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_ENQUEUED] = texture;
        ram_service->request(vfs, &ram_texture_io, &texture->ram_request, &texture->ram_destination);
        // 2. aux_fread & aux_decode -> vram_service


        // 3. direct storage
#else
        SKR_UNIMPLEMENTED_FUNCTION();
#endif

    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
    return texture;
}

void SGDIRenderer_RenderGraph::free_texture(SGDITextureId tex) SKR_NOEXCEPT
{
    auto texture = static_cast<SGDITexture_RenderGraph*>(tex);
    // TODO: cancellation
    while (texture->get_state() != EGDIResourceState::Okay) 
    {
        // wait creation...
    }
    skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Finalizing));
    if (texture->texture_view) cgpu_free_texture_view(texture->texture_view);
    if (texture->texture) cgpu_free_texture(texture->texture);
    if (texture->bind_table) cgpux_free_bind_table(texture->bind_table);
    SkrDelete(texture);
}

} }
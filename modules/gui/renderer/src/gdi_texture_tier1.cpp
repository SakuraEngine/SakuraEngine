#include "SkrGuiRenderer/gdi_renderer.hpp"
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

skr_blob_t image_coder_decode_image(uint8_t* bytes, uint64_t size, uint32_t& out_height, uint32_t& out_width, uint32_t& out_depth, ECGPUFormat& out_format)
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
        {
            const auto bit_depth = coder->get_bit_depth();
            out_depth = 1;
            out_height = coder->get_height();
            out_width = coder->get_width();
            out_format = cgpu_format_from_image_coder_format(format, raw_format, bit_depth);
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
    texture_view = cgpu_create_texture_view(async_data.device, &view_desc);

    const char* color_texture_name = "color_texture";
    CGPUXBindTableDescriptor bind_table_desc = {};
    bind_table_desc.root_signature = async_data.root_signature;
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

inline static void function_append(eastl::function<void()>& func1, const eastl::function<void()>& func2)
{
    if (func2 && func1)
    {
        auto _func1 = func1;
        func1 = [func2, _func1](){ _func1(); func2(); };
        return;
    }
    if (func2) 
        func1 = func2;
}

void SGDIImageAsyncData_RenderGraph::DoAsync(skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    auto& async_data = *this;
    auto ram_texture_io = make_zeroed<skr_ram_io_t>();
    ram_texture_io.path = async_data.uri.c_str();
    ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto pAsyncData = static_cast<SGDIImageAsyncData_RenderGraph*>(usrdata);
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
            if (pAsyncData->useImageCoder)
            {
                auto aux_task = make_zeroed<skr_service_task_t>();
                aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                    auto pAsyncData = static_cast<SGDIImageAsyncData_RenderGraph*>(usrdata);
                    pAsyncData->raw_data = image_coder_decode_image(pAsyncData->ram_destination.bytes, 
                        pAsyncData->ram_destination.size, pAsyncData->height, pAsyncData->width, pAsyncData->depth, pAsyncData->format);
                    pAsyncData->ram_data_finsihed_callback();
                };
                aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = pAsyncData;
                pAsyncData->aux_service->request(&aux_task, &pAsyncData->aux_request);
            }
            else
#endif
            {
                pAsyncData->ram_data_finsihed_callback();
            }
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = &async_data;

        ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_ENQUEUED] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto pAsyncData = static_cast<SGDIImageAsyncData_RenderGraph*>(usrdata);
            pAsyncData->ram_io_enqueued_callback();
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_ENQUEUED] = &async_data;
        ram_service->request(vfs, &ram_texture_io, &async_data.ram_request, &async_data.ram_destination);
}

void SGDITextureAsyncData_RenderGraph::DoAsync(struct SGDITexture_RenderGraph* texture, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    function_append(this->ram_io_enqueued_callback, [texture](){
        skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Loading));
    });
    function_append(this->ram_data_finsihed_callback, [texture](){
        ZoneScopedN("CreateGUITexture(AuxService)");
        auto vram_io_info = make_zeroed<skr_vram_texture_io_t>();
        const skr_blob_t& raw_data = texture->async_data.raw_data;

        vram_io_info.src_memory.bytes = raw_data.bytes;
        vram_io_info.src_memory.size = raw_data.size;
        vram_io_info.device = texture->async_data.device;
        vram_io_info.transfer_queue = texture->async_data.transfer_queue;

        vram_io_info.vtexture.depth = texture->async_data.depth;
        vram_io_info.vtexture.height = texture->async_data.height;
        vram_io_info.vtexture.width = texture->async_data.width;
        vram_io_info.vtexture.format = texture->async_data.format;
        vram_io_info.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;

        vram_io_info.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
            texture->texture = texture->async_data.vram_destination.texture;
            texture->intializeBindTable();

            skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Okay));
            if (texture->async_data.raw_data.bytes != texture->async_data.ram_destination.bytes)
            {
                sakura_free(texture->async_data.raw_data.bytes); // free image_coder decoded data
            }
            sakura_free(texture->async_data.ram_destination.bytes);
            texture->async_data.raw_data = {};
        };
        vram_io_info.callback_datas[SKR_ASYNC_IO_STATUS_OK] = texture;
        texture->async_data.vram_service->request(&vram_io_info, &texture->async_data.vram_request, &texture->async_data.vram_destination);
    });
    this->SGDIImageAsyncData_RenderGraph::DoAsync(vfs, ram_service);
}

SGDIImageId SGDIRenderer_RenderGraph::create_image(const SGDIImageDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const SGDIImageDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2->useImageCoder;

    auto image = SkrNew<SGDIImage_RenderGraph>();
    image->source = desc->source;
    image->async_data.uri = desc->from_file.u8Uri;
    image->async_data.aux_service = aux_service;
    image->async_data.useImageCoder = useImageCoder;
    image->async_data.DoAsync(vfs, ram_service);

    return image;
}

SGDITextureId SGDIRenderer_RenderGraph::create_texture(const SGDITextureDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const SGDITextureDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2->useImageCoder;

    auto texture = SkrNew<SGDITexture_RenderGraph>();
    texture->source = desc->source;
    texture->async_data.uri = desc->from_file.u8Uri;
    texture->async_data.aux_service = aux_service;
    texture->async_data.vram_service = vram_service;
    texture->async_data.device = device;
    texture->async_data.transfer_queue = transfer_queue;
    texture->async_data.root_signature = texture_pipeline->root_signature;
    texture->async_data.useImageCoder = useImageCoder;
    texture->async_data.DoAsync(texture, vfs, ram_service);

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
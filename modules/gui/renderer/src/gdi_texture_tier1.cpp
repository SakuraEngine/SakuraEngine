#include "SkrGuiRenderer/gdi_renderer.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/defer.hpp"
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
#include "SkrImageCoder/skr_image_coder.h"
#endif

namespace skr {
namespace gdi {
// start helpers
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
    SKR_UNREACHABLE_CODE();
    return CGPU_FORMAT_UNDEFINED;
}

skr_blob_t image_coder_decode_image(const uint8_t* bytes, uint64_t size, uint32_t& out_height, uint32_t& out_width, uint32_t& out_depth, ECGPUFormat& out_format)
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
// end helpers

EGDIResourceState SGDIImage_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomic32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

SGDIRendererId SGDIImage_RenderGraph::get_renderer() const SKR_NOEXCEPT
{
    return renderer;
}

uint32_t SGDIImage_RenderGraph::get_width() const SKR_NOEXCEPT
{
    return image_width;
}

uint32_t SGDIImage_RenderGraph::get_height() const SKR_NOEXCEPT
{
    return image_height;
}

LiteSpan<const uint8_t> SGDIImage_RenderGraph::get_data() const SKR_NOEXCEPT
{
    return { pixel_data.bytes, pixel_data.size };
}

EGDIImageFormat SGDIImage_RenderGraph::get_format() const SKR_NOEXCEPT
{
    switch (format)
    {
    case CGPU_FORMAT_R8G8B8A8_UNORM: return EGDIImageFormat::RGBA8;
    case CGPU_FORMAT_R8G8B8A8_SNORM: return EGDIImageFormat::RGBA8;
    case CGPU_FORMAT_R8G8B8A8_UINT: return EGDIImageFormat::RGBA8;
    case CGPU_FORMAT_R8G8B8A8_SINT: return EGDIImageFormat::RGBA8;
    case CGPU_FORMAT_R8G8B8A8_SRGB: return EGDIImageFormat::RGBA8;
    
    case CGPU_FORMAT_R8G8B8_UNORM: return EGDIImageFormat::RGB8;
    case CGPU_FORMAT_R8G8B8_SNORM: return EGDIImageFormat::RGB8;
    case CGPU_FORMAT_R8G8B8_UINT: return EGDIImageFormat::RGB8;
    case CGPU_FORMAT_R8G8B8_SINT: return EGDIImageFormat::RGB8;
    case CGPU_FORMAT_R8G8B8_SRGB: return EGDIImageFormat::RGB8;

    case CGPU_FORMAT_R8_UNORM: return EGDIImageFormat::R8;
    case CGPU_FORMAT_R8_SNORM: return EGDIImageFormat::R8;
    case CGPU_FORMAT_R8_UINT: return EGDIImageFormat::R8;
    case CGPU_FORMAT_R8_SINT: return EGDIImageFormat::R8;
    case CGPU_FORMAT_R8_SRGB: return EGDIImageFormat::R8;

    default:
        break;
    }
    return EGDIImageFormat::None;
}

EGDIResourceState SGDITexture_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomic32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

SGDIRendererId SGDITexture_RenderGraph::get_renderer() const SKR_NOEXCEPT
{
    return renderer;
}

uint32_t SGDITexture_RenderGraph::get_width() const SKR_NOEXCEPT
{
    return texture->width;
}

uint32_t SGDITexture_RenderGraph::get_height() const SKR_NOEXCEPT
{
    return texture->height;
}

EGDITextureType SGDITexture_RenderGraph::get_type() const SKR_NOEXCEPT
{
    return EGDITextureType::Texture2D;
}

void SGDIRenderer_RenderGraph::free_image(SGDIImageId img) SKR_NOEXCEPT
{
    auto image = static_cast<SGDIImage_RenderGraph*>(img);
    // TODO: cancellation
    while (image->get_state() != EGDIResourceState::Okay) 
    {
        // wait creation...
    }
    skr_atomic32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Finalizing));
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
}

SGDIImageId SGDIImageAsyncData_RenderGraph::DoAsync(struct SGDIImage_RenderGraph* owner, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    // image data must have an owner
    SKR_ASSERT(owner);

    function_append(this->ram_io_enqueued_callback, [image = owner](){
        skr_atomic32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Loading));
    });
    function_append(this->ram_io_finished_callback, [image = owner](){
        skr_atomic32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
    });
    function_append(this->ram_data_finsihed_callback, [image = owner](){
        skr_atomic32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Okay));
    });
    
    if (owner->source == EGDIImageSource::File)
    {
        auto ram_texture_io = make_zeroed<skr_ram_io_t>();
        ram_texture_io.path = from_file.uri.c_str();
        ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto owner = static_cast<SGDIImage_RenderGraph*>(usrdata);
            owner->async_data.ram_io_finished_callback();
    #ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
            if (owner->async_data.useImageCoder)
            {
                auto aux_task = make_zeroed<skr_service_task_t>();
                aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                    auto owner = static_cast<SGDIImage_RenderGraph*>(usrdata);
                    auto pAsyncData = &owner->async_data;
                    owner->pixel_data = image_coder_decode_image(owner->raw_data.bytes, 
                        owner->raw_data.size, owner->image_height, 
                        owner->image_width, owner->image_depth, owner->format);
                    pAsyncData->ram_data_finsihed_callback();
                };
                aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = owner;
                owner->async_data.aux_service->request(&aux_task, &owner->async_data.aux_request);
            }
            else
    #endif
            {
                owner->pixel_data = { owner->raw_data.bytes, owner->raw_data.size };
                owner->async_data.ram_data_finsihed_callback();
            }
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = owner;
        ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_ENQUEUED] = +[](skr_async_request_t* request, void* usrdata)
        {
            auto pAsyncData = static_cast<SGDIImageAsyncData_RenderGraph*>(usrdata);
            pAsyncData->ram_io_enqueued_callback();
        };
        ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_ENQUEUED] = this;
        ram_service->request(vfs, &ram_texture_io, &ram_request, &owner->raw_data);
    }
    else if (owner->source == EGDIImageSource::Data)
    {
        owner->async_data.ram_io_enqueued_callback();
        owner->pixel_data.bytes = owner->raw_data.bytes;
        owner->pixel_data.size = owner->raw_data.size;
        owner->async_data.ram_io_finished_callback();
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
        if (owner->async_data.useImageCoder)
        {
            owner->pixel_data = image_coder_decode_image(owner->raw_data.bytes, owner->raw_data.size,
                owner->image_height, owner->image_width, owner->image_depth, owner->format);
        }
        else
#endif
        {
            owner->image_width = from_data.width;
            owner->image_height = from_data.height;
            owner->image_depth = 1u;
            owner->pixel_data.bytes = owner->raw_data.bytes;
        }
        owner->async_data.ram_data_finsihed_callback();
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return owner;
}

SGDITextureId SGDITextureAsyncData_RenderGraph::DoAsync(struct SGDITexture_RenderGraph* owner, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    // texture data must have an owner
    SKR_ASSERT(owner);

    auto vram_request_from_image = [texture = owner](){
            ZoneScopedN("CreateGUITexture(VRAMService)");

            auto vram_io_info = make_zeroed<skr_vram_texture_io_t>();
            auto& intermediate_image = texture->intermediate_image;

            const auto& pixel_data = intermediate_image.pixel_data;
            vram_io_info.src_memory.bytes = pixel_data.bytes;
            vram_io_info.src_memory.size = pixel_data.size;
            vram_io_info.device = texture->async_data.device;
            vram_io_info.transfer_queue = texture->async_data.transfer_queue;

            vram_io_info.vtexture.depth = intermediate_image.image_depth;
            vram_io_info.vtexture.height = intermediate_image.image_height;
            vram_io_info.vtexture.width = intermediate_image.image_width;
            vram_io_info.vtexture.format = intermediate_image.format;
            vram_io_info.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;

            vram_io_info.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata)
            {
                auto texture = static_cast<SGDITexture_RenderGraph*>(usrdata);
                auto& intermediate_image = texture->intermediate_image;
                texture->texture = texture->async_data.vram_destination.texture;
                texture->intializeBindTable();

                skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Okay));
                if (intermediate_image.pixel_data.bytes != intermediate_image.raw_data.bytes)
                {
                    sakura_free(intermediate_image.pixel_data.bytes); // free image_coder decoded data
                }
                if (intermediate_image.raw_data.bytes)
                {
                    sakura_free(intermediate_image.raw_data.bytes);
                } 
                intermediate_image.pixel_data = {};
            };
            vram_io_info.callback_datas[SKR_ASYNC_IO_STATUS_OK] = texture;
            texture->async_data.vram_service->request(&vram_io_info, &texture->async_data.vram_request, &texture->async_data.vram_destination);
        };

    if (owner->source == EGDITextureSource::File || owner->source == EGDITextureSource::Data)
    {
        auto& image_async_data = owner->intermediate_image.async_data;
        function_append(image_async_data.ram_io_enqueued_callback, [texture = owner](){
            skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Loading));
        });
        function_append(image_async_data.ram_io_finished_callback, [texture = owner](){
            skr_atomic32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
        });
        function_append(image_async_data.ram_data_finsihed_callback,  vram_request_from_image);
        // ram + vram
        owner->intermediate_image.async_data.DoAsync(&owner->intermediate_image, vfs, ram_service); 
        // direct vram storage (TBD)
    }
    else if (owner->source == EGDITextureSource::Image)
    {
        skr_atomic32_store_release(&owner->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
        vram_request_from_image();
        skr_atomic32_store_release(&owner->state, static_cast<uint32_t>(EGDIResourceState::Okay));
    }
    return owner;
}

void SGDIImage_RenderGraph::preInit(const SGDIImageDescriptor* desc)
{
    if (source == EGDIImageSource::File)
    {
        source = EGDIImageSource::File;
        async_data.from_file.uri = desc->from_file.u8Uri;
    }
    else if (source == EGDIImageSource::Data)
    {
        source = EGDIImageSource::Data;
        async_data.from_data.width = desc->from_data.w;
        async_data.from_data.height = desc->from_data.h;
        async_data.from_data.format = desc->format;
        auto dst = raw_data.bytes = (uint8_t*)sakura_malloc(desc->from_data.size);
        memcpy(dst, desc->from_data.data, desc->from_data.size);
        raw_data.size = desc->from_data.size;
    }
}

SGDIImageId SGDIRenderer_RenderGraph::create_image(const SGDIImageDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const SGDIImageDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2->useImageCoder;

    auto image = SkrNew<SGDIImage_RenderGraph>(this);
    image->async_data.aux_service = aux_service;
    image->async_data.useImageCoder = useImageCoder;

    image->source = desc->source;
    if (image->source == EGDIImageSource::File)
    {
        image->async_data.from_file.uri = desc->from_file.u8Uri;
    }
    else if (image->source == EGDIImageSource::Data)
    {
        image->async_data.from_data.format = desc->format;
        image->async_data.from_data.width = desc->from_data.w;
        image->async_data.from_data.height = desc->from_data.h;
        auto dst = image->raw_data.bytes = (uint8_t*)sakura_malloc(desc->from_data.size);
        memcpy(dst, desc->from_data.data, desc->from_data.size);
        image->raw_data.size = desc->from_data.size;
    }
    return image->async_data.DoAsync(image, vfs, ram_service);
}

SGDITextureId SGDIRenderer_RenderGraph::create_texture(const SGDITextureDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const SGDITextureDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2->useImageCoder;

    auto texture = SkrNew<SGDITexture_RenderGraph>(this);
    texture->intermediate_image.async_data.aux_service = aux_service;
    texture->intermediate_image.async_data.useImageCoder = useImageCoder;

    texture->source = desc->source;
    texture->async_data.vram_service = vram_service;
    texture->async_data.device = device;
    texture->async_data.transfer_queue = transfer_queue;
    texture->async_data.root_signature = texture_pipeline->root_signature;

    if (texture->source == EGDITextureSource::Image)
    {
        texture->intermediate_image = *static_cast<SGDIImage_RenderGraph*>(desc->from_image.image);
    }
    else if (texture->source == EGDITextureSource::File)
    {
        texture->intermediate_image.source = EGDIImageSource::File;
        texture->intermediate_image.async_data.from_file.uri = desc->from_file.u8Uri;
    }
    else if (texture->source == EGDITextureSource::Data)
    {
        texture->intermediate_image.source = EGDIImageSource::Data;
        texture->intermediate_image.async_data.from_data.width = desc->from_data.w;
        texture->intermediate_image.async_data.from_data.height = desc->from_data.h;
        texture->intermediate_image.async_data.from_data.format = desc->format;
        auto dst = texture->intermediate_image.raw_data.bytes = (uint8_t*)sakura_malloc(desc->from_data.size);
        memcpy(dst, desc->from_data.data, desc->from_data.size);
        texture->intermediate_image.raw_data.size = desc->from_data.size;
    }
    return texture->async_data.DoAsync(texture, vfs, ram_service);
}

} }
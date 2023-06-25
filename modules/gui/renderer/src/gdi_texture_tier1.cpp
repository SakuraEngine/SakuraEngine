#include "futures.hpp"

#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
#include "SkrImageCoder/skr_image_coder.h"
#endif

#include "tracy/Tracy.hpp"

namespace skr {
namespace gdi {

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

skr::BlobId image_coder_decode_image(const uint8_t* bytes, uint64_t size, uint32_t& out_height, uint32_t& out_width, uint32_t& out_depth, ECGPUFormat& out_format)
{
    ZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)bytes, size);
    auto decoder = skr::IImageDecoder::Create(format);
    if (decoder->initialize((const uint8_t*)bytes, size))
    {
        SKR_DEFER({ decoder.reset(); });
        const auto encoded_format = decoder->get_color_format();
        const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
        {
            const auto bit_depth = decoder->get_bit_depth();
            out_depth = 1;
            out_height = decoder->get_height();
            out_width = decoder->get_width();
            out_format = cgpu_format_from_image_coder_format(format, raw_format, bit_depth);
        }
        decoder->decode(raw_format, decoder->get_bit_depth());
        return decoder;
    }
    return {};
}
#endif

struct DecodingProgress : public skr::AsyncProgress<ImageTexFutureLauncher, int, bool>
{
    DecodingProgress(GDIImage_RenderGraph* image)
        : owner(image)
    {

    }
    ~DecodingProgress()
    {

    }
    GDIImage_RenderGraph* owner = nullptr;

#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
    bool do_in_background() override
    {
        auto pAsyncData = &owner->async_data;
        owner->pixel_data = image_coder_decode_image(owner->raw_data->get_data(), 
            owner->raw_data->get_size(), owner->image_height, 
            owner->image_width, owner->image_depth, owner->format);
        pAsyncData->ram_data_finsihed_callback();
        return true;
    }
#else
    bool do_in_background() override {  SKR_ASSERT(0 && "ImageCoder not supportted or disabled at this platform!"); return true; }
#endif
};

// start helpers
ECGPUFormat TranslateFormat(EGDIImageFormat format)
{
    switch (format)
    {
    case EGDIImageFormat::RGB8: return CGPU_FORMAT_R8G8B8_UNORM;
    case EGDIImageFormat::RGBA8: return CGPU_FORMAT_R8G8B8A8_UNORM;
    case EGDIImageFormat::LA8: return CGPU_FORMAT_R8G8_UNORM;
    case EGDIImageFormat::R8: return CGPU_FORMAT_R8_UNORM;
    default: break;
    }
    SKR_UNREACHABLE_CODE();
    return CGPU_FORMAT_UNDEFINED;
}

void GDITexture_RenderGraph::intializeBindTable() SKR_NOEXCEPT
{
    const auto texInfo = texture->info;
    CGPUTextureViewDescriptor view_desc = {};
    view_desc.texture = texture;
    view_desc.format = texInfo->format;
    view_desc.array_layer_count = 1;
    view_desc.base_array_layer = 0;
    view_desc.mip_level_count = 1;
    view_desc.base_mip_level = 0;
    view_desc.aspects = CGPU_TVA_COLOR;
    view_desc.dims = CGPU_TEX_DIMENSION_2D;
    view_desc.usages = CGPU_TVU_SRV;
    texture_view = cgpu_create_texture_view(async_data.device, &view_desc);

    const char8_t* color_texture_name = u8"color_texture";
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

EGDIResourceState GDIImage_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomicu32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

GDIRendererId GDIImage_RenderGraph::get_renderer() const SKR_NOEXCEPT
{
    return renderer;
}

uint32_t GDIImage_RenderGraph::get_width() const SKR_NOEXCEPT
{
    return image_width;
}

uint32_t GDIImage_RenderGraph::get_height() const SKR_NOEXCEPT
{
    return image_height;
}

LiteSpan<const uint8_t> GDIImage_RenderGraph::get_data() const SKR_NOEXCEPT
{
    return { pixel_data->get_data(), pixel_data->get_size() };
}

EGDIImageFormat GDIImage_RenderGraph::get_format() const SKR_NOEXCEPT
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

    case CGPU_FORMAT_R8G8_UNORM: return EGDIImageFormat::LA8;
    case CGPU_FORMAT_R8G8_SNORM: return EGDIImageFormat::LA8;
    case CGPU_FORMAT_R8G8_UINT: return EGDIImageFormat::LA8;
    case CGPU_FORMAT_R8G8_SINT: return EGDIImageFormat::LA8;
    case CGPU_FORMAT_R8G8_SRGB: return EGDIImageFormat::LA8;

    default:
        break;
    }
    return EGDIImageFormat::None;
}

EGDIResourceState GDITexture_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomicu32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

GDIRendererId GDITexture_RenderGraph::get_renderer() const SKR_NOEXCEPT
{
    return renderer;
}

uint32_t GDITexture_RenderGraph::get_width() const SKR_NOEXCEPT
{
    return texture ? texture->info->width : 1;
}

uint32_t GDITexture_RenderGraph::get_height() const SKR_NOEXCEPT
{
    return texture ? texture->info->height : 1;
}

EGDITextureType GDITexture_RenderGraph::get_type() const SKR_NOEXCEPT
{
    return EGDITextureType::Texture2D;
}

void GDIRenderer_RenderGraph::free_image(GDIImageId img) SKR_NOEXCEPT
{
    auto image = static_cast<GDIImage_RenderGraph*>(img);
    // TODO: cancellation
    while (image->get_state() != EGDIResourceState::Okay) 
    {
        // wait creation...
    }
    skr_atomicu32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Finalizing));
    SkrDelete(img);
}

void GDIRenderer_RenderGraph::free_texture(GDITextureId tex) SKR_NOEXCEPT
{
    auto texture = static_cast<GDITexture_RenderGraph*>(tex);
    // TODO: cancellation
    while (texture->get_state() != EGDIResourceState::Okay) 
    {
        // wait creation...
    }
    skr_atomicu32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Finalizing));
    if (texture->texture_view) cgpu_free_texture_view(texture->texture_view);
    if (texture->texture) cgpu_free_texture(texture->texture);
    if (texture->bind_table) cgpux_free_bind_table(texture->bind_table);
    SkrDelete(tex);
}

EGDIResourceState GDITextureUpdate_RenderGraph::get_state() const SKR_NOEXCEPT
{
    const auto result = skr_atomicu32_load_relaxed(&state);
    return static_cast<EGDIResourceState>(result);
}

void GDIRenderer_RenderGraph::free_texture_update(IGDITextureUpdate* tex) SKR_NOEXCEPT
{
    auto update = static_cast<GDITextureUpdate_RenderGraph*>(tex);
    // TODO: cancellation
    while (update->get_state() != EGDIResourceState::Okay) 
    {
        // wait creation...
    }
    skr_atomicu32_store_release(&update->state, static_cast<uint32_t>(EGDIResourceState::Finalizing));
}

GDIImageId GDIImageAsyncData_RenderGraph::DoAsync(struct GDIImage_RenderGraph* owner, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    // image data must have an owner
    SKR_ASSERT(owner);

    function_append(this->ram_io_enqueued_callback, [image = owner](){
        skr_atomicu32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Loading));
    });
    function_append(this->ram_io_finished_callback, [image = owner](){
        skr_atomicu32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
    });
    function_append(this->ram_data_finsihed_callback, [image = owner](){
        skr_atomicu32_store_release(&image->state, static_cast<uint32_t>(EGDIResourceState::Okay));
    });
    
    if (owner->source == EGDIImageSource::File)
    {
        const auto on_complete = +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata)
        {
            auto owner = static_cast<GDIImage_RenderGraph*>(usrdata);
            owner->async_data.ram_io_finished_callback();
    #ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
            if (owner->async_data.useImageCoder)
            {
                auto progress = owner->async_data.decoding_progress = SPtr<DecodingProgress>::Create(owner);
                if (auto launcher = owner->renderer->get_future_launcher())
                {
                    progress->execute(*launcher);
                }
            }
            else
    #endif
            {
                owner->pixel_data = owner->raw_data;
                owner->async_data.ram_data_finsihed_callback();
                // owner->format
            }
        };
        const auto on_enqueue = +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata)
        {
            auto pAsyncData = static_cast<GDIImageAsyncData_RenderGraph*>(usrdata);
            pAsyncData->ram_io_enqueued_callback();
        };
        auto rq = ram_service->open_request();
        rq->set_vfs(vfs);
        rq->set_path(from_file.uri.u8_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED, on_complete, owner);
        rq->add_callback(SKR_IO_STAGE_ENQUEUED, on_enqueue, this);
        owner->raw_data = ram_service->request(rq, &ram_request);
    }
    else if (owner->source == EGDIImageSource::Data)
    {
        owner->async_data.ram_io_enqueued_callback();
        owner->pixel_data = owner->raw_data;
        owner->async_data.ram_io_finished_callback();
#ifdef SKR_GUI_RENDERER_USE_IMAGE_CODER
        if (owner->async_data.useImageCoder)
        {
            owner->pixel_data = image_coder_decode_image(owner->raw_data->get_data(), owner->raw_data->get_size(),
                owner->image_height, owner->image_width, owner->image_depth, owner->format);
        }
        else
#endif
        {
            owner->image_width = from_data.width;
            owner->image_height = from_data.height;
            owner->image_depth = 1u;
            owner->pixel_data = owner->raw_data;
            owner->format = TranslateFormat(from_data.gdi_format);
        }
        owner->async_data.ram_data_finsihed_callback();
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return owner;
}

GDITextureId GDITextureAsyncData_RenderGraph::DoAsync(struct GDITexture_RenderGraph* owner, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT
{
    // texture data must have an owner
    SKR_ASSERT(owner);

    auto vram_request_from_image = [texture = owner](){
            ZoneScopedN("CreateGUITexture(VRAMService)");

            auto vram_io_info = make_zeroed<skr_vram_texture_io_t>();
            auto& intermediate_image = texture->intermediate_image;

            const auto& pixel_data = intermediate_image.pixel_data;
            vram_io_info.src_memory.bytes = pixel_data->get_data();
            vram_io_info.src_memory.size = pixel_data->get_size();
            vram_io_info.device = texture->async_data.device;
            vram_io_info.transfer_queue = texture->async_data.transfer_queue;

            vram_io_info.vtexture.depth = intermediate_image.image_depth;
            vram_io_info.vtexture.height = intermediate_image.image_height;
            vram_io_info.vtexture.width = intermediate_image.image_width;
            vram_io_info.vtexture.format = intermediate_image.format;
            vram_io_info.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;

            vram_io_info.callbacks[SKR_IO_STAGE_COMPLETED] = +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata)
            {
                auto texture = static_cast<GDITexture_RenderGraph*>(usrdata);
                auto& intermediate_image = texture->intermediate_image;
                texture->texture = texture->async_data.vram_destination.texture;
                texture->intializeBindTable();

                skr_atomicu32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Okay));
                
                intermediate_image.pixel_data.reset();
                intermediate_image.raw_data.reset();
            };
            vram_io_info.callback_datas[SKR_IO_STAGE_COMPLETED] = texture;
            texture->async_data.vram_service->request(&vram_io_info, &texture->async_data.vram_request, &texture->async_data.vram_destination);
        };

    if (owner->source == EGDITextureSource::File || owner->source == EGDITextureSource::Data)
    {
        auto& image_async_data = owner->intermediate_image.async_data;
        function_append(image_async_data.ram_io_enqueued_callback, [texture = owner](){
            skr_atomicu32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Loading));
        });
        function_append(image_async_data.ram_io_finished_callback, [texture = owner](){
            skr_atomicu32_store_release(&texture->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
        });
        function_append(image_async_data.ram_data_finsihed_callback,  vram_request_from_image);
        // ram + vram
        owner->intermediate_image.async_data.DoAsync(&owner->intermediate_image, vfs, ram_service); 
        // direct vram storage (TBD)
    }
    else if (owner->source == EGDITextureSource::Image)
    {
        skr_atomicu32_store_release(&owner->state, static_cast<uint32_t>(EGDIResourceState::Initializing));
        vram_request_from_image();
    }
    return owner;
}

void GDIImage_RenderGraph::preInit(const GDIImageDescriptor* desc)
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
        async_data.from_data.gdi_format = desc->format;
        raw_data = skr::IBlob::Create(desc->from_data.data, desc->from_data.size, false);
    }
}

GDIImageId GDIRenderer_RenderGraph::create_image(const GDIImageDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const GDIImageDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2 ? desc2->useImageCoder : false;

    auto image = SkrNew<GDIImage_RenderGraph>(this);
    image->async_data.useImageCoder = useImageCoder;

    image->source = desc->source;
    if (image->source == EGDIImageSource::File)
    {
        image->async_data.from_file.uri = desc->from_file.u8Uri;
    }
    else if (image->source == EGDIImageSource::Data)
    {
        image->async_data.from_data.gdi_format = desc->format;
        image->async_data.from_data.width = desc->from_data.w;
        image->async_data.from_data.height = desc->from_data.h;
        image->raw_data = skr::IBlob::Create(desc->from_data.data, desc->from_data.size, false);
    }
    return image->async_data.DoAsync(image, vfs, ram_service);
}

GDITextureId GDIRenderer_RenderGraph::create_texture(const GDITextureDescriptor* desc) SKR_NOEXCEPT
{
    auto desc2 = static_cast<const GDITextureDescriptor_RenderGraph*>(desc->usr_data);
    const bool useImageCoder = desc2 ? desc2->useImageCoder : false;

    auto texture = SkrNew<GDITexture_RenderGraph>(this);
    texture->intermediate_image.async_data.useImageCoder = useImageCoder;

    texture->source = desc->source;
    texture->async_data.vram_service = vram_service;
    texture->async_data.device = device;
    texture->async_data.transfer_queue = transfer_queue;
    // this pipeline definately exists
    PipelineKey key = { GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED, CGPU_SAMPLE_COUNT_1 };
    texture->async_data.root_signature = pipelines[key]->root_signature;

    if (texture->source == EGDITextureSource::Image)
    {
        texture->intermediate_image = *static_cast<GDIImage_RenderGraph*>(desc->from_image.image);
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
        texture->intermediate_image.async_data.from_data.gdi_format = desc->format;
        texture->intermediate_image.raw_data = skr::IBlob::Create(desc->from_data.data, desc->from_data.size, false);
    }
    return texture->async_data.DoAsync(texture, vfs, ram_service);
}

} }
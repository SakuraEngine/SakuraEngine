#include "SkrGuiRenderer/resource/skr_image_task.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrImageCoder/skr_image_coder.h"
#include "SkrProfile/profile.h"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrRT/misc/make_zeroed.hpp"

// decode
namespace skr::gui
{
ECGPUFormat _cgpu_format_from_image_coder_format(EImageCoderFormat format, EImageCoderColorFormat cformat, uint32_t bit_depth) SKR_NOEXCEPT
{
    (void)bit_depth;
    if (format == IMAGE_CODER_FORMAT_JPEG || format == IMAGE_CODER_FORMAT_PNG)
    {
        switch (cformat)
        {
            case IMAGE_CODER_COLOR_FORMAT_RGBA:
                return CGPU_FORMAT_R8G8B8A8_UNORM;
            case IMAGE_CODER_COLOR_FORMAT_BGRA:
                return CGPU_FORMAT_B8G8R8A8_UNORM;
            case IMAGE_CODER_COLOR_FORMAT_Gray:
                return CGPU_FORMAT_R8_UNORM;

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

skr::BlobId _image_coder_decode_image(const uint8_t* bytes, uint64_t size, uint32_t& out_height, uint32_t& out_width, uint32_t& out_depth, ECGPUFormat& out_format)
{
    SkrZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format  = skr_image_coder_detect_format((const uint8_t*)bytes, size);
    auto              decoder = skr::IImageDecoder::Create(format);
    if (decoder->initialize((const uint8_t*)bytes, size))
    {
        SKR_DEFER({ decoder.reset(); });
        const auto encoded_format = decoder->get_color_format();
        const auto raw_format     = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
        {
            const auto bit_depth = decoder->get_bit_depth();
            out_depth            = 1;
            out_height           = decoder->get_height();
            out_width            = decoder->get_width();
            out_format           = _cgpu_format_from_image_coder_format(format, raw_format, bit_depth);
        }
        decoder->decode(raw_format, decoder->get_bit_depth());
        return decoder;
    }
    return {};
}

struct DecodingProgress : public skr::AsyncProgress<skr::FutureLauncher<bool>, int, bool> {
    DecodingProgress(SkrImageDataTask* image)
        : owner(image)
    {
    }
    ~DecodingProgress()
    {
    }
    SkrImageDataTask* owner = nullptr;

    bool do_in_background() override
    {
        uint32_t height, width;
        owner->_image_data._pixel_data = _image_coder_decode_image(owner->_raw_data->get_data(),
                                                                   owner->_raw_data->get_size(),
                                                                   height,
                                                                   width,
                                                                   owner->_image_data._image_depth,
                                                                   owner->_image_data._format);
        owner->_async_trans_state(SkrImageDataTask::EState::Okey);
        return true;
    }
};
} // namespace skr::gui

namespace skr::gui
{
SkrImageData::SkrImageData(EPixelFormat format, Sizei size, Span<const uint8_t> data, bool move_data)
    : _format(pixel_format_to_cgpu_format(format))
    , _size(size)
    , _pixel_data(IBlob::Create(data.data(), data.size(), move_data))
{
}

SkrImageDataTask::SkrImageDataTask(SkrResourceDevice* resource_service)
    : _owner(resource_service)
{
}

SkrImageDataTask::EState SkrImageDataTask::state() const SKR_NOEXCEPT
{
    return (EState)skr_atomicu32_load_relaxed((uint32_t*)&_state);
}

void SkrImageDataTask::from_file(StringView file_path, bool need_decode)
{
    _need_decode = need_decode;

    auto ram_service = _owner->ram_service();
    auto vfs         = _owner->vfs();

    auto rq = ram_service->open_request();
    rq->set_vfs(vfs);
    rq->set_path(file_path.raw().data());
    rq->add_block({}); // read all
    rq->add_callback(
    SKR_IO_STAGE_ENQUEUED, +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) {
        auto self = reinterpret_cast<SkrImageDataTask*>(usrdata);
        self->_async_trans_state(EState::Loading);
    },
    this);
    rq->add_callback(
    SKR_IO_STAGE_COMPLETED, +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) {
        auto self = reinterpret_cast<SkrImageDataTask*>(usrdata);
        self->_async_trans_state(EState::Initializing);
        self->_async_decode_data();
    },
    this);
    _raw_data = ram_service->request(rq, &_ram_request);
}

void SkrImageDataTask::from_data(Span<const uint8_t> data)
{
    _need_decode = true;
    _raw_data    = IBlob::Create(data.data(), data.size(), false);
    _async_trans_state(EState::Initializing);
    _async_decode_data();
}

void SkrImageDataTask::_async_trans_state(EState target)
{
    skr_atomicu32_store_release((uint32_t*)&_state, (uint32_t)target);
}

void SkrImageDataTask::_async_decode_data()
{
    if (_need_decode)
    {
        _decoding_progress = SPtr<DecodingProgress>::Create(this);
        if (auto launcher = _owner->future_launcher())
        {
            _decoding_progress->execute(*launcher);
        }
    }
    else
    {
        _image_data.pixel_data() = _raw_data;
        _async_trans_state(EState::Okey);
    }
}

SkrImageUploadTask::SkrImageUploadTask(SkrRenderDevice* render_device)
    : _render_device(render_device)
{
    // PipelineKey key = { GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED, CGPU_SAMPLE_COUNT_1 };
    // texture->async_data.root_signature = pipelines[key]->root_signature;
}

bool SkrImageUploadTask::is_okey()
{
    return skr_atomicu32_load_relaxed(&_async_is_okey);
}

void SkrImageUploadTask::from_image(const SkrImageData& image)
{
    _image_data = image;

    SkrZoneScopedN("CreateGUITexture(VRAMService)");

    auto vram_service = _render_device->vram_service();
    auto request = vram_service->open_texture_request();
    const auto& pixel_data        = _image_data.pixel_data();
    CGPUTextureDescriptor tdesc = {};
    tdesc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
    tdesc.depth          = _image_data.image_depth();
    tdesc.height         = _image_data.size().height;
    tdesc.width          = _image_data.size().width;
    tdesc.format         = _image_data.cgpu_format();
    request->set_transfer_queue(_render_device->cgpu_queue());
    request->set_memory_src(pixel_data->get_data(), pixel_data->get_size());
    request->set_texture(_render_device->cgpu_device(), &tdesc);
    request->add_callback(SKR_IO_STAGE_COMPLETED, +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) {
        auto task = static_cast<SkrImageUploadTask*>(usrdata);

        auto        device     = task->_render_device->cgpu_device();
        const auto& image_data = task->_image_data;

        task->_texture                      = task->_vram_destination->get_texture();
        CGPUTextureViewDescriptor view_desc = {};
        view_desc.texture                   = task->_texture;
        view_desc.format                    = image_data.cgpu_format();
        view_desc.array_layer_count         = 1;
        view_desc.base_array_layer          = 0;
        view_desc.mip_level_count           = 1;
        view_desc.base_mip_level            = 0;
        view_desc.aspects                   = CGPU_TVA_COLOR;
        view_desc.dims                      = CGPU_TEX_DIMENSION_2D;
        view_desc.usages                    = CGPU_TVU_SRV;
        task->_texture_view                 = cgpu_create_texture_view(device, &view_desc);

        const char8_t*           color_texture_name = u8"color_texture";
        CGPUXBindTableDescriptor bind_table_desc    = {};
        bind_table_desc.root_signature              = task->_root_signature;
        bind_table_desc.names                       = &color_texture_name;
        bind_table_desc.names_count                 = 1;
        task->_bind_table                           = cgpux_create_bind_table(device, &bind_table_desc);
        auto data                                   = make_zeroed<CGPUDescriptorData>();
        data.name                                   = color_texture_name;
        data.count                                  = 1;
        data.binding_type                           = CGPU_RESOURCE_TYPE_TEXTURE;
        data.textures                               = &task->_texture_view;
        cgpux_bind_table_update(task->_bind_table, &data, 1);

        skr_atomicu32_store_release(&task->_async_is_okey, 1);
    }, this);
    _vram_destination = vram_service->request(request, &_ram_request);
}

} // namespace skr::gui
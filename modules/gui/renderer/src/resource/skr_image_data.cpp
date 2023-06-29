#include "SkrGuiRenderer/resource/skr_image_data.hpp"
#include "SkrGuiRenderer/resource/skr_resource_service.hpp"
#include "SkrImageCoder/skr_image_coder.h"
#include "futures.hpp"

// decode
namespace skr::gui
{
ECGPUFormat TranslateFormat(EPixelFormat format)
{
    switch (format)
    {
        case EPixelFormat::RGB8:
            return CGPU_FORMAT_R8G8B8_UNORM;
        case EPixelFormat::RGBA8:
            return CGPU_FORMAT_R8G8B8A8_UNORM;
        case EPixelFormat::L8:
            return CGPU_FORMAT_R8_UNORM;
        case EPixelFormat::LA8:
            return CGPU_FORMAT_R8G8_UNORM;
        case EPixelFormat::R8:
            return CGPU_FORMAT_R8_UNORM;
        default:
            break;
    }
    SKR_UNREACHABLE_CODE();
    return CGPU_FORMAT_UNDEFINED;
}

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
    ZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)bytes, size);
    auto              decoder = skr::IImageDecoder::Create(format);
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
            out_format = _cgpu_format_from_image_coder_format(format, raw_format, bit_depth);
        }
        decoder->decode(raw_format, decoder->get_bit_depth());
        return decoder;
    }
    return {};
}
struct DecodingProgress : public skr::AsyncProgress<skr::FutureLauncher<bool>, int, bool> {
    DecodingProgress(SkrImageData* image)
        : owner(image)
    {
    }
    ~DecodingProgress()
    {
    }
    SkrImageData* owner = nullptr;

    bool do_in_background() override
    {
        uint32_t height, width;
        owner->_pixel_data = _image_coder_decode_image(owner->_raw_data->get_data(),
                                                       owner->_raw_data->get_size(),
                                                       height,
                                                       width,
                                                       owner->_image_depth,
                                                       owner->_format);
        owner->_async_trans_state(SkrImageData::EState::Okey);
        return true;
    }
};
} // namespace skr::gui

namespace skr::gui
{
SkrImageData::SkrImageData(SkrResourceService* resource_service)
    : _owner(resource_service)
{
}

SkrImageData::EState SkrImageData::state() const SKR_NOEXCEPT
{
    return (EState)skr_atomicu32_load_relaxed((uint32_t*)&_state);
}
void SkrImageData::from_file(StringView file_path, bool need_decode)
{
    _need_decode = need_decode;

    auto ram_service = _owner->ram_service();
    auto vfs = _owner->vfs();

    auto rq = ram_service->open_request();
    rq->set_vfs(vfs);
    rq->set_path(file_path.u8_str());
    rq->add_block({}); // read all
    rq->add_callback(
    SKR_IO_STAGE_ENQUEUED, +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) {
        auto self = reinterpret_cast<SkrImageData*>(usrdata);
        self->_async_trans_state(EState::Loading);
    },
    this);
    rq->add_callback(
    SKR_IO_STAGE_COMPLETED, +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) {
        auto self = reinterpret_cast<SkrImageData*>(usrdata);
        self->_async_trans_state(EState::Initializing);
        self->_async_decode_data();
    },
    this);
    _raw_data = ram_service->request(rq, &_ram_request);
}
void SkrImageData::from_data(Span<const uint8_t> data)
{
    _need_decode = true;
    _raw_data = IBlob::Create(data.data(), data.size(), false);
    _async_trans_state(EState::Initializing);
    _async_decode_data();
}
void SkrImageData::from_decoded_data(EPixelFormat format, Sizei size, Span<const uint8_t> data)
{
    _need_decode = false;
    _format = TranslateFormat(format);
    _size = size;
    _pixel_data = IBlob::Create(data.data(), data.size(), false);
    _async_trans_state(EState::Okey);
}

void SkrImageData::_async_trans_state(EState target)
{
    skr_atomicu32_store_release((uint32_t*)&_state, (uint32_t)target);
}
void SkrImageData::_async_decode_data()
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
        _pixel_data = _raw_data;
        _async_trans_state(EState::Okey);
    }
}

} // namespace skr::gui
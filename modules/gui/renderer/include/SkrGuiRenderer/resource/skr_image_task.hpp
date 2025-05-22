#pragma once
#include "SkrCore/memory/sp.hpp"
#include "SkrGraphics/cgpux.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "SkrBase/config.h"
#include "SkrGui/backend/resource/resource.hpp"

// format helper
namespace skr::gui
{
inline ECGPUFormat pixel_format_to_cgpu_format(EPixelFormat format)
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
        default:
            break;
    }
    SKR_UNREACHABLE_CODE();
    return CGPU_FORMAT_UNDEFINED;
}
inline EPixelFormat cgpu_format_to_pixel_format(ECGPUFormat format)
{
    switch (format)
    {
        case CGPU_FORMAT_R8G8B8_UNORM:
            return EPixelFormat::RGB8;
        case CGPU_FORMAT_R8G8B8A8_UNORM:
            return EPixelFormat::RGBA8;
        case CGPU_FORMAT_R8_UNORM:
            return EPixelFormat::L8;
        case CGPU_FORMAT_R8G8_UNORM:
            return EPixelFormat::LA8;
        default:
            break;
    }
    SKR_UNREACHABLE_CODE();
    return EPixelFormat::Unknown;
}

} // namespace skr::gui

namespace skr::gui
{
struct SkrResourceDevice;
struct DecodingProgress;
struct SkrUpdatableImage;
struct SkrRenderDevice;

struct SKR_GUI_RENDERER_API SkrImageData {
    inline SkrImageData() = default;
    SkrImageData(EPixelFormat format, Sizei size, Span<const uint8_t> data, bool move_data = false);

    inline ECGPUFormat  cgpu_format() const SKR_NOEXCEPT { return _format; }
    inline EPixelFormat format() const SKR_NOEXCEPT { return cgpu_format_to_pixel_format(_format); }
    inline Sizei        size() const SKR_NOEXCEPT { return _size; }
    inline uint32_t     image_depth() const SKR_NOEXCEPT { return _image_depth; }
    inline BlobId       pixel_data() const SKR_NOEXCEPT { return _pixel_data; }

    ECGPUFormat _format      = CGPU_FORMAT_UNDEFINED;
    Sizei       _size        = {};
    uint32_t    _image_depth = 0;
    BlobId      _pixel_data  = nullptr;
};

struct SKR_GUI_RENDERER_API SkrImageDataTask {
    enum class EState : uint32_t
    {
        Requested,
        Loading,
        Initializing,
        Okey,
    };

    SkrImageDataTask(SkrResourceDevice* resource_service);

    EState state() const SKR_NOEXCEPT;
    void   from_file(StringView file_path, bool need_decode);
    void   from_data(Span<const uint8_t> data);

    // getter
    inline const SkrImageData& image_data() const SKR_NOEXCEPT { return _image_data; }
    inline BlobId              raw_data() const SKR_NOEXCEPT { return _raw_data; }

private:
    // help function
    void _async_trans_state(EState target);
    void _async_decode_data();

private:
    friend struct DecodingProgress;
    SkrResourceDevice* _owner = nullptr;

    // data
    SkrImageData _image_data = {};
    BlobId       _raw_data   = nullptr;

    // async
    EState                 _state             = EState::Requested;
    skr_io_future_t        _ram_request       = {};
    SP<DecodingProgress> _decoding_progress = nullptr;
    bool                   _need_decode       = false;
};

struct SKR_GUI_RENDERER_API SkrImageUploadTask {
    SkrImageUploadTask(SkrRenderDevice* render_device);

    bool is_okey();
    void from_image(const SkrImageData& data);

private:
    SkrRenderDevice* _render_device = nullptr;

    // data
    SkrImageData        _image_data     = {};
    CGPUTextureId       _texture        = nullptr;
    CGPUTextureViewId   _texture_view   = nullptr;
    CGPUXBindTableId    _bind_table     = nullptr;
    CGPURootSignatureId _root_signature = nullptr;

    // async
    skr_io_future_t                  _ram_request      = {};
    skr::io::VRAMIOTextureId         _vram_destination = {};
    uint32_t                         _async_is_okey    = false;
};

} // namespace skr::gui
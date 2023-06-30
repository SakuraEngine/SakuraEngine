#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "cgpu/io.h"
#include "SkrGui/backend/resource/resource.hpp"

namespace skr::gui
{
struct SkrResourceService;
struct DecodingProgress;
struct SkrUpdatableImage;

struct SKR_GUI_RENDERER_API SkrImageDataTask {
    enum class EState : uint32_t
    {
        Requested,
        Loading,
        Initializing,
        Okey,
    };

    SkrImageDataTask(SkrResourceService* resource_service);

    EState state() const SKR_NOEXCEPT;
    void   from_file(StringView file_path, bool need_decode);
    void   from_data(Span<const uint8_t> data);
    void   from_decoded_data(EPixelFormat format, Sizei size, Span<const uint8_t> data);

private:
    // help function
    void _async_trans_state(EState target);
    void _async_decode_data();

private:
    friend struct DecodingProgress;
    SkrResourceService* _owner = nullptr;

    // data
    ECGPUFormat _format = CGPU_FORMAT_UNDEFINED;
    Sizei       _size = {};
    uint32_t    _image_depth = 0;
    BlobId      _pixel_data = nullptr;
    BlobId      _raw_data = nullptr;

    // async
    EState                 _state = EState::Requested;
    skr_io_future_t        _ram_request = {};
    SPtr<DecodingProgress> _decoding_progress = nullptr;
    bool                   _need_decode = false;
};

struct SKR_GUI_RENDERER_API SkrImageUploadTask {
    SkrImageUploadTask(SkrResourceService* resource_service);

    bool is_okey();

private:
    SkrResourceService* _owner = nullptr;

    // data
    SkrUpdatableImage* _update_image = nullptr;
    SkrImageDataTask*  _image_data_task = nullptr;

    // async
    skr_async_vtexture_destination_t _async_destination = {};
    uint32_t                         _async_is_okey = false;
};

} // namespace skr::gui
#pragma once
#include "containers/span.hpp"
#include "utils/types.h"
#include "skr_image_coder/skr_image_coder.h"

namespace skr
{
class SKR_IMAGE_CODER_API BaseImageCoder : public skr_image_coder_t
{
public:
    virtual ~BaseImageCoder() SKR_NOEXCEPT;

    virtual uint64_t get_raw_size() const SKR_NOEXCEPT override;
    virtual uint64_t get_encoded_size() const SKR_NOEXCEPT override;

    virtual bool set_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT override;
    virtual bool move_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT override;
    virtual bool view_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT override;
    virtual bool set_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height,
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT override;
    virtual bool move_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height,
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT override;
    virtual bool view_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height,
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT override;

    virtual bool get_raw_data(uint8_t* pData, uint64_t* pSize, EImageCoderColorFormat format, uint32_t bit_depth) const SKR_NOEXCEPT override;
    virtual bool get_encoded_data(uint8_t* pData, uint64_t* pSize) const SKR_NOEXCEPT override;
    virtual skr::span<const uint8_t> get_raw_data_view(EImageCoderColorFormat format, uint32_t bit_depth) const SKR_NOEXCEPT override;
    virtual skr::span<const uint8_t> get_encoded_data_view() const SKR_NOEXCEPT override;

    virtual EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT override;
    virtual uint32_t get_width() const SKR_NOEXCEPT override;
    virtual uint32_t get_height() const SKR_NOEXCEPT override;
    virtual uint32_t get_bit_depth() const SKR_NOEXCEPT override;

    virtual bool decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT = 0;
    virtual bool encode() SKR_NOEXCEPT = 0;

protected:
    bool lazy_decode(EImageCoderColorFormat format, uint32_t bit_depth) const SKR_NOEXCEPT;
    bool lazy_encode() const SKR_NOEXCEPT;

    skr::span<uint8_t> raw_view;
    skr::span<uint8_t> encoded_view;

    uint32_t width = 0;
    uint32_t height = 0;
    EImageCoderColorFormat color_format;
    uint8_t bit_depth = 0;

    uint8_t raw_bit_depth = 0;
    uint32_t raw_bytes_per_row = 0;

private:
    void freeRaw() SKR_NOEXCEPT;
    void freeEncoded() SKR_NOEXCEPT;
    void setRawProps(uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT;

    skr_blob_t raw_data;
    mutable uint32_t raw_version = 0;
    skr_blob_t encoded_data;
    mutable uint32_t encoded_version = 0;
};
} // namespace skr
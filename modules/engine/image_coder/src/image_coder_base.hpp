#pragma once
#include "SkrBase/atomic/atomic.h"
#include "SkrContainers/span.hpp"
#include "SkrImageCoder/skr_image_coder.h"

namespace skr
{

struct ImageProps {
    uint32_t               width  = 0;
    uint32_t               height = 0;
    EImageCoderColorFormat color_format;
    uint8_t                bit_depth = 0;
};

struct SKR_IMAGE_CODER_API BaseImageEncoder : public skr::IImageEncoder {
    SKR_RC_IMPL(override)
    SKR_RC_DELETER_IMPL_DEFAULT(override)
    virtual ~BaseImageEncoder() SKR_NOEXCEPT;

    virtual uint64_t get_alignment() const SKR_NOEXCEPT { return alignof(uint8_t); }
    static uint8_t*  Allocate(uint64_t size, uint64_t alignment) SKR_NOEXCEPT;
    static void      Deallocate(uint8_t* ptr, uint64_t alignment) SKR_NOEXCEPT;

    virtual bool initialize(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT override;

    skr::span<const uint8_t> decoded_view;

    virtual uint8_t* get_data() const SKR_NOEXCEPT override { return encoded_data; }
    virtual uint64_t get_size() const SKR_NOEXCEPT override { return encoded_size; }
    uint8_t*         encoded_data = nullptr;
    uint64_t         encoded_size = 0;

    virtual EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT override { return props.color_format; }
    virtual uint32_t               get_width() const SKR_NOEXCEPT override { return props.width; }
    virtual uint32_t               get_height() const SKR_NOEXCEPT override { return props.height; }
    virtual uint32_t               get_bit_depth() const SKR_NOEXCEPT override { return props.bit_depth; }
    void                           setRawProps(uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT;
    uint32_t                       bytes_per_row = 0;

protected:
    bool       initialized = false;
    ImageProps props;

public:
    uint32_t add_refcount()
    {
        return 1 + skr_atomic_fetch_add_relaxed(&rc, 1);
    }
    uint32_t release()
    {
        skr_atomic_fetch_add_relaxed(&rc, -1);
        return skr_atomic_load_acquire(&rc);
    }

private:
    SAtomicU32 rc = 0;
};

struct SKR_IMAGE_CODER_API BaseImageDecoder : public skr::IImageDecoder {
    SKR_RC_IMPL(override)
    SKR_RC_DELETER_IMPL_DEFAULT(override)

    virtual ~BaseImageDecoder() SKR_NOEXCEPT;

    virtual uint64_t get_alignment() const SKR_NOEXCEPT { return alignof(uint8_t); }
    static uint8_t*  Allocate(uint64_t size, uint64_t alignment) SKR_NOEXCEPT;
    static void      Deallocate(uint8_t* ptr, uint64_t alignment) SKR_NOEXCEPT;

    virtual bool initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT override;

    skr::span<const uint8_t> encoded_view;

    virtual uint8_t* get_data() const SKR_NOEXCEPT override { return decoded_data; }
    virtual uint64_t get_size() const SKR_NOEXCEPT override { return decoded_size; }
    uint8_t*         decoded_data = nullptr;
    uint64_t         decoded_size = 0;

    virtual EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT override { return props.color_format; }
    virtual uint32_t               get_width() const SKR_NOEXCEPT override { return props.width; }
    virtual uint32_t               get_height() const SKR_NOEXCEPT override { return props.height; }
    virtual uint32_t               get_bit_depth() const SKR_NOEXCEPT override { return props.bit_depth; }
    void                           setRawProps(uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT;

protected:
    bool       initialized = false;
    ImageProps props;

public:
    uint32_t add_refcount()
    {
        return 1 + skr_atomic_fetch_add_relaxed(&rc, 1);
    }
    uint32_t release()
    {
        skr_atomic_fetch_add_relaxed(&rc, -1);
        return skr_atomic_load_acquire(&rc);
    }

private:
    SAtomicU32 rc = 0;
};

} // namespace skr
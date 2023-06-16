#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API IGDIResource {
    virtual ~IGDIResource() SKR_NOEXCEPT = default;
    virtual EGDIResourceState get_state() const SKR_NOEXCEPT = 0;
};
struct SKR_GUI_API IGDIImage : public IGDIResource {
    virtual ~IGDIImage() SKR_NOEXCEPT = default;
    virtual IGDIRenderer*       get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t            get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t            get_height() const SKR_NOEXCEPT = 0;
    virtual Span<const uint8_t> get_data() const SKR_NOEXCEPT = 0;
    virtual EGDIImageFormat     get_format() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API IGDITextureUpdate : public IGDIResource {
    virtual ~IGDITextureUpdate() SKR_NOEXCEPT = default;
};

struct SKR_GUI_API IGDITexture : public IGDIResource {
    virtual ~IGDITexture() SKR_NOEXCEPT = default;
    virtual IGDIRenderer* get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t      get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t      get_height() const SKR_NOEXCEPT = 0;

    virtual EGDITextureType get_type() const SKR_NOEXCEPT = 0;
};

} // namespace skr::gdi
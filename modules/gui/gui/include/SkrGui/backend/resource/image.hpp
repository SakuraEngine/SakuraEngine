#pragma once
#include "SkrGui/backend/resource/surfaces.hpp"

namespace skr::gui
{
struct IImageSource;
enum class EPixelFormat
{
    None,
    RGB8,
    RGBA8,
    LA8,
    R8,
};

// GDI Image 是 CPU 数据
// GDI Texture 是 GPU 纹理
// 本处设计更偏向于：
// 将 GDI Image 映射为 IImageSource
// 将 GDI Texture 映射为 IImage
struct SKR_GUI_API IImage : public ISurface {
    virtual Size          size() const SKR_NOEXCEPT = 0;
    virtual IImageSource* source() const SKR_NOEXCEPT = 0;
    virtual Rect          uv_rect() const SKR_NOEXCEPT = 0;         // [0, 1]
    virtual Rect          nine_inner_rect() const SKR_NOEXCEPT = 0; // [0, size()]
};

struct SKR_GUI_API IImageSource {
    virtual ~IImageSource() = default;
};

struct SKR_GUI_API IImageUpdateReply {
    virtual ~IImageUpdateReply() = default;
    virtual bool okey() const SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
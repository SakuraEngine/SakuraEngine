#pragma once
#include "SkrGui/backend/resource/surfaces.hpp"

namespace skr::gui
{
struct IImageSource;

// GDI Image 是 CPU 数据
// GDI Texture 是 GPU 纹理
// 本处设计更偏向于：
// 将 GDI Image 映射为 IImageSource
// 将 GDI Texture 映射为 IImage
// TODO. ResourceProvider Widget/Element
struct SKR_GUI_API IImage : public ISurface {
    virtual Sizef                  size() const SKR_NOEXCEPT = 0; // in image pixel
    virtual NotNull<IImageSource*> source() const SKR_NOEXCEPT = 0;
    virtual Rectf                  uv_rect() const SKR_NOEXCEPT = 0;         // [0, 1]
    virtual Rectf                  nine_inner_rect() const SKR_NOEXCEPT = 0; // [0, size()]
};

struct SKR_GUI_API IImageEntry {
    virtual ~IImageEntry() = default;

    virtual NotNull<IImage*> load_image_of_size(Sizef show_size) SKR_NOEXCEPT = 0;
    virtual void             release_image(NotNull<IImage*> image) SKR_NOEXCEPT = 0;

    virtual void visit_useable_image_size(FunctionRef<void(Sizef)> visitor) const SKR_NOEXCEPT = 0;
    virtual void visit_requested_image(FunctionRef<void(NotNull<IImage*>)> visitor) const SKR_NOEXCEPT = 0;
};

enum class EPixelFormat
{
    None,
    RGB8,
    RGBA8,
    LA8,
    R8,
};
struct ImageUpdateDesc {
    uint32_t      width = 0;
    uint32_t      height = 0;
    uint32_t      mip_count = 0;
    uint64_t      data_length = 0;
    SPtr<uint8_t> data = {};
};

struct SKR_GUI_API IImageEntryUpdatable : public IImageEntry {
    virtual void update(const ImageUpdateDesc& desc) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
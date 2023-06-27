#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/resource/resource.hpp"
#include "SkrGui/backend/resource/resource_entry.hpp"

// resources
namespace skr::gui
{
struct SKR_GUI_API ISurface : virtual public IResource {
    SKR_GUI_INTERFACE(ISurface, "bcdb2a40-6d70-4b4e-9b7d-c07c82da9873", IResource)
};

struct SKR_GUI_API IMaterial : virtual public ISurface {
    SKR_GUI_INTERFACE(IMaterial, "14f90556-f9ff-486e-bc33-6ee6aa4f535d", ISurface)
};

struct SKR_GUI_API IImage : virtual public ISurface {
    SKR_GUI_INTERFACE(IImage, "4cc917b5-2e84-491d-87ab-9f6b74c2c44c", ISurface)
    virtual Sizef size() const SKR_NOEXCEPT = 0;            // in image pixel
    virtual Rectf uv_rect() const SKR_NOEXCEPT = 0;         // [0, 1]
    virtual Rectf nine_inner_rect() const SKR_NOEXCEPT = 0; // [0, size()]
};
} // namespace skr::gui

// entries
namespace skr::gui
{
struct SKR_GUI_API IImageEntry : public IResourceEntry {
    SKR_GUI_INTERFACE(IImageEntry, "8964753f-bea2-48cd-9eae-fe9de69b77a3", IResourceEntry)

    virtual NotNull<IImage*> request_image_of_size(Sizef show_size) SKR_NOEXCEPT = 0;

    virtual void visit_useable_image_size(FunctionRef<void(Sizef)> visitor) const SKR_NOEXCEPT = 0;
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

struct SKR_GUI_API IUpdatableImageEntry : public IImageEntry {
    virtual void update(const ImageUpdateDesc& desc) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
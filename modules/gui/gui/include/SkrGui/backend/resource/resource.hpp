#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

// def
namespace skr::gui
{
struct IResource;
struct IUpdatableImage;
} // namespace skr::gui

// resource service
namespace skr::gui
{
enum class EPixelFormat
{
    Unknown,
    RGB8,
    RGBA8,
    L8,
    LA8,
};

struct UpdatableImageDesc {
    EPixelFormat        format = EPixelFormat::Unknown;
    Sizei               size = {};
    uint32_t            mip_count = 0;
    Span<const uint8_t> data = {};
};

struct SKR_GUI_API IResourceService SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResourceService, "4d8c09d2-2b06-40c2-979a-213b3f1e08e6")
    virtual ~IResourceService() = default;

    virtual NotNull<IUpdatableImage*> create_updatable_image(const UpdatableImageDesc& desc) = 0;
    virtual void                      destroy_resource(NotNull<IResource*> resource) = 0;
};
} // namespace skr::gui

// resource
namespace skr::gui
{
struct SKR_GUI_API IResource SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResource, "26ede0ec-cd41-420b-ba29-893ab06bfce9")
    virtual ~IResource() = default;

    virtual bool is_okey() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API ISurface : virtual public IResource {
    SKR_GUI_INTERFACE(ISurface, "bcdb2a40-6d70-4b4e-9b7d-c07c82da9873", IResource)
};

struct SKR_GUI_API IMaterial : virtual public ISurface {
    SKR_GUI_INTERFACE(IMaterial, "14f90556-f9ff-486e-bc33-6ee6aa4f535d", ISurface)
};

struct SKR_GUI_API IImage : virtual public ISurface {
    SKR_GUI_INTERFACE(IImage, "4cc917b5-2e84-491d-87ab-9f6b74c2c44c", ISurface)
    virtual Sizei       size() const SKR_NOEXCEPT = 0;       // in image pixel
    virtual Rectf       uv_rect() const SKR_NOEXCEPT = 0;    // [0, 1]
    virtual EdgeInsetsf nine_inset() const SKR_NOEXCEPT = 0; // [0, size()]
};

struct SKR_GUI_API IUpdatableImage : virtual public IImage {
    SKR_GUI_INTERFACE(IUpdatableImage, "e82f5c9e-3a79-4d86-99df-7e949a41fbdf", IImage)
    virtual void                      update(const UpdatableImageDesc& desc) = 0;
    virtual const UpdatableImageDesc& desc() const SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
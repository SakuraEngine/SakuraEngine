#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

// def
namespace skr::gui
{
struct IResourceEntry;
}

// resource & resource entry
namespace skr::gui
{
enum class EResourceState
{
    Requested,    // 资源请求已提交
    Loading,      // 资源正在加载
    Initializing, // 资源正在初始化
    Okay,         // 资源已就绪（本帧是否可以使用）
    Destroying,   // 资源正在销毁
    Destroyed,    // 资源已销毁
};

struct SKR_GUI_API IResource SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResource, "26ede0ec-cd41-420b-ba29-893ab06bfce9")
    virtual ~IResource() = default;

    virtual EResourceState           state() const SKR_NOEXCEPT = 0;
    virtual NotNull<IResourceEntry*> entry() const SKR_NOEXCEPT = 0;
};

using VisitResourceFuncRef = FunctionRef<void(NotNull<IResource*>)>;

struct SKR_GUI_API IResourceEntry SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResourceEntry, "2664f750-8c30-4f0b-b5d8-4161f0744312")
    virtual ~IResourceEntry() = default;

    virtual void destroy_resoure(NotNull<IResource*> resource) SKR_NOEXCEPT = 0;
    virtual void visit_requested_resources(VisitResourceFuncRef visitor) = 0;
};

} // namespace skr::gui

// surface resource
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

// surface entries
namespace skr::gui
{
struct SKR_GUI_API IImageEntry : public IResourceEntry {
    SKR_GUI_INTERFACE(IImageEntry, "8964753f-bea2-48cd-9eae-fe9de69b77a3", IResourceEntry)

    virtual NotNull<IImage*> request_image_of_size(Sizef show_size) SKR_NOEXCEPT = 0;

    virtual void visit_useable_image_size(FunctionRef<void(Sizef)> visitor) const SKR_NOEXCEPT = 0;
};

enum class EPixelFormat
{
    Unknown,
    RGB8,
    RGBA8,
    L8,
    LA8,
    R8,
};

struct SKR_GUI_API IUpdatableImageEntry : public IImageEntry {
    struct UpdateDesc {
        EPixelFormat  format = EPixelFormat::Unknown;
        Sizei         size = {};
        uint32_t      mip_count = 0;
        Span<uint8_t> data = {};
    };
    virtual void update(const UpdateDesc& desc) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
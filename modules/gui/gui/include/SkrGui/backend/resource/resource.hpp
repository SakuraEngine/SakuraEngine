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
    EPixelFormat        format    = EPixelFormat::Unknown;
    Sizei               size      = {};
    uint32_t            mip_count = 0;
    Span<const uint8_t> data      = {};
};

enum class EResourceState : uint32_t
{
    Entry,          // 入口状态，知晓部分资产信息，但是还未被加载
    Requested,      // 已经申请加载，但是还没进入正式的加载流程，这个阶段是可以取消加载的
    Loading,        // 正在加载，这个阶段是不可以取消加载的
    Okey,           // 加载完成
    Failed,         // 加载失败，调用 Request 重新请求加载
    PendingDestroy, // 资源正在等待释放，此时调用 Request 可以保住资源并直接进入 Okey 状态
    Destroyed,      // 资源已经被释放，此时再调用 Request 必须重新加载
};

struct SKR_GUI_API IResource SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResource, "26ede0ec-cd41-420b-ba29-893ab06bfce9")
    virtual ~IResource() = default;

    // resource state
    virtual EResourceState state() const SKR_NOEXCEPT = 0;
    virtual void           request()                  = 0;
    virtual void           cancel_request()           = 0;
    virtual void           destroy()                  = 0;
};

struct SKR_GUI_API ISurface : virtual public IResource {
    SKR_GUI_INTERFACE(ISurface, "bcdb2a40-6d70-4b4e-9b7d-c07c82da9873", IResource)
};

struct SKR_GUI_API IMaterial : virtual public ISurface {
    SKR_GUI_INTERFACE(IMaterial, "14f90556-f9ff-486e-bc33-6ee6aa4f535d", ISurface)
};

struct SKR_GUI_API IImage : virtual public ISurface {
    SKR_GUI_INTERFACE(IImage, "4cc917b5-2e84-491d-87ab-9f6b74c2c44c", ISurface)
    virtual Sizei       size() const SKR_NOEXCEPT       = 0; // in image pixel
    virtual Rectf       uv_rect() const SKR_NOEXCEPT    = 0; // [0, 1]
    virtual EdgeInsetsf nine_inset() const SKR_NOEXCEPT = 0; // [0, size()]
};

struct SKR_GUI_API IUpdatableImage : virtual public IImage {
    SKR_GUI_INTERFACE(IUpdatableImage, "e82f5c9e-3a79-4d86-99df-7e949a41fbdf", IImage)
    virtual void                      update(const UpdatableImageDesc& desc) = 0;
    virtual const UpdatableImageDesc& desc() const SKR_NOEXCEPT              = 0;
};
} // namespace skr::gui
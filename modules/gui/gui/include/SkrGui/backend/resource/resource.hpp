#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/backend/resource/resource.generated.h"
#endif

// def
namespace skr::gui
{
struct IResource;
struct IUpdatableImage;
} // namespace skr::gui

// resource service
namespace skr sreflect
{
namespace gui sreflect
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

sreflect_struct(
    "guid": "1d4ba81f-09b7-4186-b35a-f380c49302e4"
)
SKR_GUI_API IResource : virtual skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
    virtual ~IResource() = default;

    // resource state
    virtual EResourceState state() const SKR_NOEXCEPT = 0;
    virtual void           request()                  = 0;
    virtual void           cancel_request()           = 0;
    virtual void           destroy()                  = 0;
};

sreflect_struct(
    "guid": "f0a63d5a-62ae-44fa-8f8f-6847af623cea"
)
SKR_GUI_API ISurface : virtual public IResource {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "41d08a49-c9fe-4ccb-a91d-fd16f946aca1"
)
SKR_GUI_API IMaterial : virtual public ISurface {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct(
    "guid": "6cc1395f-9660-4431-b998-df32d1d363eb"
)
SKR_GUI_API IImage : virtual public ISurface {
    SKR_RTTR_GENERATE_BODY()
    virtual Sizei       size() const SKR_NOEXCEPT       = 0; // in image pixel
    virtual Rectf       uv_rect() const SKR_NOEXCEPT    = 0; // [0, 1]
    virtual EdgeInsetsf nine_inset() const SKR_NOEXCEPT = 0; // [0, size()]
};

sreflect_struct(
    "guid": "7ae28a98-10f2-44c4-b7aa-b50780435d03"
)
SKR_GUI_API IUpdatableImage : virtual public IImage {
    SKR_RTTR_GENERATE_BODY()
    virtual void                      update(const UpdatableImageDesc& desc) = 0;
    virtual const UpdatableImageDesc& desc() const SKR_NOEXCEPT              = 0;
};
} // namespace gui sreflect
} // namespace skr sreflect
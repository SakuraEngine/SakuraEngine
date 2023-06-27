#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

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

struct IResourceEntry;

struct SKR_GUI_API IResource SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResource, "26ede0ec-cd41-420b-ba29-893ab06bfce9")
    virtual ~IResource() = default;

    virtual EResourceState  state() const SKR_NOEXCEPT = 0;
    virtual IResourceEntry* entry() const SKR_NOEXCEPT = 0;
};

} // namespace skr::gui
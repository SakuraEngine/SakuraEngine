#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
// 三颗控件树的总控制器，管理所有控件树的 update/layout/paint
// 同时提供环境 InputManager/WindowManager
struct SKR_GUI_API BuildOwner final {
    BuildOwner(NotNull<INativeDevice*> native_device) SKR_NOEXCEPT;

    // schedule
    void schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;
    void schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT;

    // flush
    void flush_layout();
    void flush_paint();

    inline INativeDevice* native_device() const SKR_NOEXCEPT { return _native_device; }

    // build
    void schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT;
    void flush_build() SKR_NOEXCEPT;

    // TODO. temporal impl for pass compile, move to global context
    inline void drop_unmount_element(NotNull<Element*> element) SKR_NOEXCEPT {}

private:
    Array<Element*>      _dirty_elements;
    Array<RenderObject*> _nodes_needing_layout = {};
    Array<RenderObject*> _nodes_needing_paint  = {};

    INativeDevice* _native_device = nullptr;
};
} // namespace skr::gui
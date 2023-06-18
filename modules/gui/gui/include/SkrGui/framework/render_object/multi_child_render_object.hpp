#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API IMultiChildRenderObject SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IMultiChildRenderObject, "e244fce1-ff1c-4fb7-b51e-bd4cc81a659f")
    virtual ~IMultiChildRenderObject() = default;

    virtual uint32_t child_count() const SKR_NOEXCEPT = 0;

    virtual void begin_rebuild_children() SKR_NOEXCEPT = 0;
    virtual void rebuild_move_child(uint32_t from, uint32_t to) SKR_NOEXCEPT = 0;
    virtual void rebuild_set_child(uint32_t index, RenderObject* child) SKR_NOEXCEPT = 0;
    virtual void end_rebuild_children() SKR_NOEXCEPT = 0;
};
} // namespace skr::gui
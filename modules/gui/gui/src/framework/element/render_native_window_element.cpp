#include "SkrGui/framework/element/render_native_window_element.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
void RenderNativeWindowElement::add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    auto obj = render_object()->type_cast_fast<ISingleChildRenderObject>();
    obj->set_child(child);
}
void RenderNativeWindowElement::remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    auto obj = render_object()->type_cast_fast<ISingleChildRenderObject>();
    obj->remove_child();
}
void RenderNativeWindowElement::move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT
{
    SKR_UNREACHABLE_CODE()
}
void RenderNativeWindowElement::visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT
{
    if (_child)
    {
        visitor(make_not_null(_child));
    }
}

void RenderNativeWindowElement::prepare_initial_frame() SKR_NOEXCEPT
{
    // TODO. mount(nullptr)
}
} // namespace skr::gui
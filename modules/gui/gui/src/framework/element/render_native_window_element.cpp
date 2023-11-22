#include "SkrGui/framework/element/render_native_window_element.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/widget/render_native_window_widget.hpp"
#include "SkrGui/framework/render_object/render_native_window.hpp"

namespace skr::gui
{
void RenderNativeWindowElement::perform_rebuild() SKR_NOEXCEPT
{
    if (_new_child_widget)
    {
        auto cache_widget = _new_child_widget;
        _new_child_widget = nullptr;
        update(cache_widget);
    }
    Super::perform_rebuild();
}
void RenderNativeWindowElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    _rebuild();
}

void RenderNativeWindowElement::add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    // auto              raw_obj = render_object();
    // constexpr int64_t offset  = skr::rttr::get_cast_offset<RenderNativeWindow, ISingleChildRenderObject>();
    // auto              obj     = static_cast<ISingleChildRenderObject*>(static_cast<RenderNativeWindow*>(raw_obj));
    auto obj = render_object()->type_cast<ISingleChildRenderObject>();
    obj->set_child(child);
}
void RenderNativeWindowElement::remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    auto obj = render_object()->type_cast<ISingleChildRenderObject>();
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
        visitor(_child);
    }
}

void RenderNativeWindowElement::set_new_child_widget(Widget* widget) SKR_NOEXCEPT
{
    _new_child_widget = widget;
    mark_needs_build();
}

void RenderNativeWindowElement::_rebuild()
{
    _child = _update_child(_child, widget()->type_cast_fast<RenderNativeWindowWidget>()->child, Slot::Invalid());
}

void RenderNativeWindowElement::prepare_initial_frame() SKR_NOEXCEPT
{
    // fake mount
    if (_lifecycle == EElementLifecycle::Initial)
    {
        _render_object = widget()->type_cast_fast<RenderNativeWindowWidget>()->create_render_object();

        _rebuild();

        // recursive call attach()
        struct _RecursiveHelper {
            NotNull<BuildOwner*> owner;

            bool operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
                return true;
            }
        };
        this->visit_children(_RecursiveHelper{ _owner });
    }
    _lifecycle = EElementLifecycle::Mounted;
}
} // namespace skr::gui
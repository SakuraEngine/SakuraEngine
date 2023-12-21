#include "SkrGui/framework/build_owner.hpp"
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/framework/element/render_native_window_element.hpp"

namespace skr::gui
{
BuildOwner::BuildOwner(NotNull<INativeDevice*> native_device) SKR_NOEXCEPT
    : _native_device(native_device)
{
}

// schedule
void BuildOwner::schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT
{
    _dirty_elements.add(element);
}
void BuildOwner::schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT
{
    _nodes_needing_layout.add(node);
}
void BuildOwner::schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT
{
    _nodes_needing_paint.add(node);
}

// flush
void BuildOwner::flush_build() SKR_NOEXCEPT
{
    if (_dirty_elements.size() == 0) return;

    // sort by depth and is_dirty
    std::sort(
    _dirty_elements.begin(),
    _dirty_elements.end(),
    +[](Element* a, Element* b) {
        return a->depth() == b->depth() ? a->is_dirty() < b->is_dirty() : a->depth() < b->depth();
    });

    // build
    for (auto element : _dirty_elements)
    {
        element->rebuild();
    }

    _dirty_elements.clear();
}
void BuildOwner::flush_layout()
{
    std::sort(
    _nodes_needing_layout.begin(),
    _nodes_needing_layout.end(),
    +[](RenderObject* a, RenderObject* b) {
        return a->depth() < b->depth();
    });

    for (auto node : _nodes_needing_layout)
    {
        if (node->needs_layout() && node->owner() == this)
        {
            node->perform_layout();
            node->_needs_layout = false;
            node->mark_needs_paint();
        }
    }

    _nodes_needing_layout.clear();
}
void BuildOwner::flush_paint()
{
    std::sort(
    _nodes_needing_paint.begin(),
    _nodes_needing_paint.end(),
    +[](RenderObject* a, RenderObject* b) {
        return a->depth() < b->depth();
    });

    for (auto node : _nodes_needing_paint)
    {
        if (node->needs_paint())
        {
            PaintingContext::repaint_composited_child(node);
        }
        else
        {
            PaintingContext::update_layer_properties(node);
        }
    }

    _nodes_needing_paint.clear();
}

// register
void BuildOwner::register_native_window(NotNull<RenderNativeWindowElement*> native_window)
{
    _native_windows.add_unique(native_window);
    if (_input_manager)
    {
        auto render_object = native_window->render_object()->type_cast_fast<RenderNativeWindow>();
        _input_manager->register_context(render_object);
    }
}
void BuildOwner::unregister_native_window(NotNull<RenderNativeWindowElement*> native_window)
{
    _native_windows.remove(native_window);
    if (_input_manager)
    {
        auto render_object = native_window->render_object()->type_cast_fast<RenderNativeWindow>();
        _input_manager->unregister_context(render_object);
    }
}
} // namespace skr::gui

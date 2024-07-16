#include "SkrGui/framework/build_context.hpp"
#include "SkrGui/framework/element/render_object_element.hpp"
#include "SkrGui/framework/element/stateful_element.hpp"
#include "SkrGui/framework/widget/stateful_widget.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
Widget* IBuildContext::find_ancestor_widget(const GUID& type_id, bool exact_type) const SKR_NOEXCEPT
{
    Widget* found_widget = nullptr;
    visit_ancestor_elements([&](NotNull<Element*> element) {
        auto widget = element->widget();
        if (exact_type)
        {
            if (widget->iobject_get_typeid() == type_id)
            {
                found_widget = widget;
            }
        }
        else
        {
            if (widget->type_is(type_id))
            {
                found_widget = widget;
            }
        }
        return !found_widget;
    });
    return found_widget;
}
State* IBuildContext::find_ancestor_state(const GUID& type_id, bool exact_type) const SKR_NOEXCEPT
{
    State* found_state = nullptr;
    visit_ancestor_elements([&](NotNull<Element*> element) {
        if (auto found_element = element->type_cast<StatefulElement>())
        {
            auto state = found_element->state();
            if (exact_type)
            {
                if (state->iobject_get_typeid() == type_id)
                {
                    found_state = state;
                }
            }
            else
            {
                if (state->type_is(type_id))
                {
                    found_state = state;
                }
            }
        }
        return !found_state;
    });
    return found_state;
}
RenderObject* IBuildContext::find_ancestor_render_object() const SKR_NOEXCEPT
{
    RenderObjectElement* found_element = nullptr;
    visit_ancestor_elements([&found_element](NotNull<Element*> element) {
        found_element = element->type_cast<RenderObjectElement>();
        return !found_element;
    });
    return found_element->render_object();
}
RenderObject* IBuildContext::find_ancestor_render_object(const GUID& type_id, bool exact_type) const SKR_NOEXCEPT
{
    RenderObject* found_render_object = nullptr;
    visit_ancestor_elements([&](NotNull<Element*> element) {
        if (auto found_element = element->type_cast<RenderObjectElement>())
        {
            auto render_object = found_element->render_object();
            if (exact_type)
            {
                if (render_object->iobject_get_typeid() == type_id)
                {
                    found_render_object = render_object;
                }
            }
            else
            {
                if (render_object->type_is(type_id))
                {
                    found_render_object = render_object;
                }
            }
        }
        return !found_render_object;
    });
    return found_render_object;
}
} // namespace skr::gui
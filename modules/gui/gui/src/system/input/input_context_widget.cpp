#include "SkrGui/system/input/input_context_widget.hpp"
#include "SkrGui/system/input/render_input_context.hpp"

namespace skr::gui
{

NotNull<RenderObject*> InputContextWidget::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderInputContext>();
    result->set_manager(manager);
    return result;
}
void InputContextWidget::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    auto render_input_context = render_object->type_cast_fast<RenderInputContext>();
    render_input_context->set_manager(manager);
}

} // namespace skr::gui
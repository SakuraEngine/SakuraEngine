#include "SkrGui/widgets/stack.hpp"
#include "SkrGui/render_objects/render_stack.hpp"

namespace skr::gui
{
NotNull<RenderObject*> Stack::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderStack>();
    result->set_stack_alignment(stack_alignment);
    result->set_child_fit(child_fit);
    result->set_stack_size(stack_size);
    return result;
}
void Stack::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    auto r_obj = render_object->type_cast_fast<RenderStack>();

    r_obj->set_stack_alignment(stack_alignment);
    r_obj->set_child_fit(child_fit);
    r_obj->set_stack_size(stack_size);
}
} // namespace skr::gui
#include "SkrGui/widgets/stack.hpp"
#include "SkrGui/render_objects/render_stack.hpp"

namespace skr::gui
{
NotNull<RenderObject*> Stack::create_render_object() SKR_NOEXCEPT
{
    auto result = make_not_null(SkrNew<RenderStack>());
    result->set_stack_alignment(stack_alignment);
    result->set_child_fit(child_fit);
    result->set_stack_size(stack_size);
    return result;
}
void Stack::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui
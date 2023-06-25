#include "SkrGui/widgets/stack.hpp"
#include "SkrGui/render_objects/render_stack.hpp"

namespace skr::gui
{
NotNull<RenderObject*> Stack::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderStack>());
}
void Stack::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui
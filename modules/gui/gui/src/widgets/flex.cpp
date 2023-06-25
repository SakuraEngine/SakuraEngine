#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/render_objects/render_flex.hpp"

namespace skr::gui
{

NotNull<RenderObject*> Flex::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderFlex>());
}
void Flex::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}

} // namespace skr::gui
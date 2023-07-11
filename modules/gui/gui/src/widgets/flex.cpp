#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/render_objects/render_flex.hpp"

namespace skr::gui
{

NotNull<RenderObject*> Flex::create_render_object() SKR_NOEXCEPT
{
    auto result = make_not_null(SkrNew<RenderFlex>());
    result->set_flex_direction(flex_direction);
    result->set_main_axis_alignment(main_axis_alignment);
    result->set_cross_axis_alignment(cross_axis_alignment);
    result->set_main_axis_size(main_axis_size);
    return result;
}
void Flex::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}

} // namespace skr::gui
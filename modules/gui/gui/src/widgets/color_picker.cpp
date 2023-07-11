#include "SkrGui/widgets/color_picker.hpp"
#include "SkrGui/render_objects/render_color_picker.hpp"

namespace skr::gui
{
NotNull<RenderObject*> ColorPicker::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderColorPicker>());
}
void ColorPicker::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui
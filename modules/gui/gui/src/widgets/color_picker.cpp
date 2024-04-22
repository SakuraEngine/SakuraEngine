#include "SkrGui/widgets/color_picker.hpp"
#include "SkrGui/render_objects/render_color_picker.hpp"

namespace skr::gui
{
NotNull<RenderObject*> ColorPicker::create_render_object() SKR_NOEXCEPT
{
    return SkrNew<RenderColorPicker>();
}
void ColorPicker::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    [[maybe_unused]] auto r_obj = render_object->type_cast_fast<RenderColorPicker>();
}
} // namespace skr::gui
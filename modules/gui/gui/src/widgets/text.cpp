#include "SkrGui/widgets/text.hpp"
#include "SkrGui/render_objects/render_text.hpp"

namespace skr::gui
{
NotNull<RenderObject*> Text::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderText>();
    result->set_text(text);
    return result;
}

void Text::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui
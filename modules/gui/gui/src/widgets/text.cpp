#include "../pch.hpp"
#include "SkrGui/widgets/text.hpp"
#include "SkrGui/render_objects/render_text.hpp"

namespace skr::gui
{
NotNull<RenderObject*> Text::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderText>());
}

void Text::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
} // namespace skr::gui
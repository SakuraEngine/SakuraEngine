#include "SkrGui/widgets/positioned.hpp"
#include "SkrGui/render_objects/render_positioned.hpp"

namespace skr::gui
{

NotNull<RenderObject*> Positioned::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderPositioned>();
    result->set_positional(positional);
    return result;
}
void Positioned::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    auto r_obj = render_object->type_cast_fast<RenderPositioned>();

    r_obj->set_positional(positional);
}

} // namespace skr::gui

#include "SkrGui/widgets/sized_box.hpp"
#include "SkrGui/render_objects/render_constrained_box.hpp"

namespace skr::gui
{

NotNull<RenderObject*> SizedBox::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderConstrainedBox>();
    result->set_additional_constraint(BoxConstraints::Tight(size));
    return result;
}
void SizedBox::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    auto r_obj = render_object->type_cast_fast<RenderConstrainedBox>();

    r_obj->set_additional_constraint(BoxConstraints::Tight(size));
}

} // namespace skr::gui

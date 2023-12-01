#include "SkrGui/widgets/mouse_region.hpp"
#include "SkrGui/render_objects/render_mouse_region.hpp"

namespace skr::gui
{
NotNull<RenderObject*> MouseRegin::create_render_object() SKR_NOEXCEPT
{
    auto result = SkrNew<RenderMouseRegion>();

    result->hit_test_behavior = hit_test_behaviour;
    result->on_enter          = std::move(on_enter);
    result->on_exit           = std::move(on_exit);
    result->on_hover          = std::move(on_hover);
    result->on_down           = std::move(on_down);
    result->on_up             = std::move(on_up);

    return result;
}
void MouseRegin::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    auto mouse_region = render_object->type_cast_fast<RenderMouseRegion>();

    mouse_region->hit_test_behavior = hit_test_behaviour;
    mouse_region->on_enter          = std::move(on_enter);
    mouse_region->on_exit           = std::move(on_exit);
    mouse_region->on_hover          = std::move(on_hover);
    mouse_region->on_down           = std::move(on_down);
    mouse_region->on_up             = std::move(on_up);
}
} // namespace skr::gui
#include "SkrGui/widgets/grid_paper.hpp"
#include "SkrGui/render_objects/render_grid_paper.hpp"

namespace skr::gui
{

NotNull<RenderObject*> GridPaper::create_render_object() SKR_NOEXCEPT
{
    return SkrNew<RenderGridPaper>();
}
void GridPaper::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
    [[maybe_unused]] auto r_obj = render_object->type_cast_fast<RenderGridPaper>();
}
} // namespace skr::gui
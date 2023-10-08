#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_grid_paper.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid" : "4207334c-617c-4f7c-bf3c-fd2f9e018a9c"
)
SKR_GUI_API RenderGridPaper : public RenderBox {
public:
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderBox;

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}
};
} // namespace gui sreflect
} // namespace skr sreflect
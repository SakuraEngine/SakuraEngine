#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_shifted_box.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// 会对 child 施加布局偏移的 RenderBox
sreflect_struct(
    "guid": "357e11e8-dbcd-4830-9256-869198ca7bed",
    "rtti": true
)
RenderShiftedBox : public RenderBox,
                   public ISingleChildRenderObject
{
    SKR_RTTR_GENERATE_BODY()

    inline Offsetf offset() const SKR_NOEXCEPT { return _offset; }
    inline void    set_offset(Offsetf offset) SKR_NOEXCEPT { _offset = offset; }

protected:
    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

private:
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Offsetf _offset = {};

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderShiftedBox, RenderBox)
};
} // namespace gui sreflect
} // namespace skr sreflect
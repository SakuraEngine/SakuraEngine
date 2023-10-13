#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_constrained_box.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "4d40a67a-1bd4-457a-bd23-b5be13e7d04d",
    "rtti": true
)
RenderConstrainedBox : public RenderProxyBox
{
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderProxyBox;

    // getter setter
    inline const BoxConstraints& additional_constraint() const SKR_NOEXCEPT { return _additional_constraint; }
    inline void                  set_additional_constraint(const BoxConstraints& constraint) SKR_NOEXCEPT
    {
        if (_additional_constraint != constraint)
        {
            _additional_constraint = constraint;
            mark_needs_layout();
        }
    }

    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

private:
    BoxConstraints _additional_constraint = {};
};
} // namespace gui sreflect
} // namespace skr sreflect
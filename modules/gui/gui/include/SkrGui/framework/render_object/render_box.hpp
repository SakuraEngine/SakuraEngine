#pragma once
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/math/layout.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_box.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "d4c45487-d696-42fb-bff1-f0a3f6adcea3",
    "rtti": true
)
SKR_GUI_API RenderBox : public RenderObject {
    SKR_RTTR_GENERATE_BODY()

public:
    RenderBox();
    ~RenderBox();

    // getter & setter
    inline Sizef          size() const SKR_NOEXCEPT { return _size; }
    inline void           set_size(Sizef size) SKR_NOEXCEPT { _size = size; }
    inline BoxConstraints constraints() const SKR_NOEXCEPT { return _constraints; }
    inline void           set_constraints(BoxConstraints constraints) SKR_NOEXCEPT
    {
        if (_constraints != constraints)
        {
            _constraints = constraints;
            _set_force_relayout_boundary(_constraints.is_tight());
            _set_constraints_changed(true);
        }
    }

    // intrinsic size
    float get_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    float get_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    Sizef get_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT;

    // hit test
    virtual bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT;

    // TODO.
    // global_to_local
    // local_to_global
    // paint_bounds
    // hit_test
    // handle_event

protected:
    // intrinsic size
    virtual float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    virtual Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT;

    // helper function
    using HitTestFuncRef = FunctionRef<bool(HitTestResult* result, Offsetf local_position)>;
    bool _default_hit_test(HitTestResult* result, Offsetf local_position, HitTestFuncRef hit_test_self, HitTestFuncRef hit_test_children) const;

private:
    void perform_resize() SKR_NOEXCEPT override; // override compute_dry_layout instead

private:
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Sizef          _size        = {};
    BoxConstraints _constraints = {};

    // TODO. cached data
};
} // namespace gui sreflect
} // namespace skr sreflect
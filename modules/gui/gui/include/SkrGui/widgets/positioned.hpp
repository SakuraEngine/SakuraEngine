#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Positioned : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(Positioned, "7084ef1f-3c12-43d9-b2b0-c6dfa2fab257", SingleChildRenderObjectWidget)

    //==> Begin Construct
    struct Params {
        // 约束定位 or 锚点定位
        PositionalUnit left = PositionalUnit::null();
        PositionalUnit top = PositionalUnit::null();
        PositionalUnit right = PositionalUnit::null();
        PositionalUnit bottom = PositionalUnit::null();

        // 锚点定位中使用的尺寸约束
        PositionalUnit min_width = PositionalUnit::null();  // as 0 if needs
        PositionalUnit max_width = PositionalUnit::null();  // as inf if needs
        PositionalUnit min_height = PositionalUnit::null(); // as o if needs
        PositionalUnit max_height = PositionalUnit::null(); // as inf if needs

        // 锚点
        Offset pivot = { 0, 0 };

        Widget* child = nullptr;
    };
    inline void construct(Params params) SKR_NOEXCEPT
    {
        _positional.left = params.left;
        _positional.top = params.top;
        _positional.right = params.right;
        _positional.bottom = params.bottom;
        _positional.min_width = params.min_width;
        _positional.max_width = params.max_width;
        _positional.min_height = params.min_height;
        _positional.max_height = params.max_height;
        _positional.pivot = params.pivot;

        _child = params.child;
    }

    struct Padding {
        PositionalUnit all = PositionalUnit::null();

        PositionalUnit horizontal = PositionalUnit::null();
        PositionalUnit vertical = PositionalUnit::null();

        PositionalUnit left = PositionalUnit::null();
        PositionalUnit top = PositionalUnit::null();
        PositionalUnit right = PositionalUnit::null();
        PositionalUnit bottom = PositionalUnit::null();

        Widget* child = nullptr;
    };
    inline void construct(Padding params) SKR_NOEXCEPT
    {
        // _positional = Positional::Padding({
        //     .all = params.all,
        //     .horizontal = params.horizontal,
        //     .vertical = params.vertical,
        //     .left = params.left,
        //     .top = params.top,
        //     .right = params.right,
        //     .bottom = params.bottom,
        // });

        _child = params.child;
    }

    struct Align {
        Offset pivot = { 0.5, 0.5 }; // percent offset
        Positional::ConstraintsParams constraints = {};
        Widget* child = nullptr;
    };
    inline void construct(Align params) SKR_NOEXCEPT
    {
        // _positional = Positional::Anchor({
        //                                      .left = params.pivot.x,
        //                                      .top = params.pivot.y,
        //                                  },
        //                                  params.constraints, params.pivot);

        _child = params.child;
    }

    struct Center {
        Positional::ConstraintsParams constraints = {};
        Widget* child = nullptr;
    };
    inline void construct(Center params) SKR_NOEXCEPT
    {
        // construct(Align{
        //     .pivot = { 0.5, 0.5 },
        //     .constraints = params.constraints,
        //     .child = params.child,
        // });
    }

    struct Fill {
        Widget* child = nullptr;
    };
    inline void construct(Fill params) SKR_NOEXCEPT
    {
        // construct(Params{
        //     .left = 0,
        //     .top = 0,
        //     .right = 0,
        //     .bottom = 0,
        //     .child = params.child,
        // });
    }
    //==> End Construct

private:
    Positional _positional;
};
} // namespace skr::gui
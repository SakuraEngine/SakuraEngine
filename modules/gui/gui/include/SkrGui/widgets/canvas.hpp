#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct Canvas : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Canvas, "94714676-422c-4e4e-a754-e26b5466900f", MultiChildRenderObjectWidget)

    struct Slot {
        Positional layout = Positional::fill();
        int32_t z_index = 0;
        Widget* widget = nullptr;
    };
    using Params = std::initializer_list<Slot>;
    void construct(Params params) SKR_NOEXCEPT;

private:
    struct _Slot {
        Positional layout = Positional::fill();
        int32_t z_index = 0;
    };
    Array<Slot> _children_slots;
};
} // namespace skr::gui
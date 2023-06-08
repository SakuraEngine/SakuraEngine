#include "SkrGui/framework/sandbox.hpp"
#include "platform/memory.h"

#include "SkrGui/framework/widget_misc.hpp"

// !!!! TestWidgets !!!!
#include "SkrGui/widgets/canvas.hpp"
#include "SkrGui/widgets/positioned.hpp"
namespace skr::gui
{
void MayBeExample()
{
    // canvas
    //   grid_paper
    //   color_picker
    //   stack
    //     flex
    //       colored_box
    //       colored_box
    //       colored_box
    //     text
    // auto sandbox = SkrNew<Sandbox>(NewWidget<Canvas>({
    //     Canvas::Slot{
    //         .layout = Positional::fill(),
    //         .z_index = 1,
    //         .widget = NewWidget<Positioned>({}),
    //     },
    //     Canvas::Slot{
    //         .layout = Positional::fill(),
    //         .z_index = 2,
    //         .widget = NewWidget<Positioned>({
    //             .child = NewWidget<Positioned>({
    //                 .left = 10,
    //                 .top = 10,
    //                 .right = 0.5_pct,
    //                 .bottom = 0.5_pct,
    //             }),
    //         }),
    //     },
    // }));

    // sandbox->update();
    // sandbox->layout();
    // sandbox->draw();
}
} // namespace skr::gui

namespace skr::gui
{
Sandbox::Sandbox(Widget* root_widget)
    : _root_widget(root_widget)
{
}

void Sandbox::update()
{
}

void Sandbox::layout()
{
}

void Sandbox::draw()
{
}

} // namespace skr::gui
#include "SkrGui/framework/sandbox.hpp"
#include "platform/memory.h"

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
    // auto sandbox = SkrNew<Sandbox>(
    // SNewWidget(Canvas) {
    //     p = SNewSlotList(Canvas)
    //     {
    //         SNewSlot(Canvas)
    //         {
    //             s.layout = Positional::fill();
    //             s.z_index = 0;
    //             s.widget = SNewWidget(Positioned::Align)
    //             {
    //                 p.pivot = { 0.5f, 0.5f };
    //                 p.child = SNewWidget(Positioned::Fill){};
    //             };
    //         };
    //     };
    // });

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
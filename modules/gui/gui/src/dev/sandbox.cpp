#include "SkrGui/dev/sandbox.hpp"
#include "platform/memory.h"

// !!!! TestWidgets !!!!
#include "SkrGui/widgets/canvas.hpp"
#include "SkrGui/widgets/color_picker.hpp"
#include "SkrGui/widgets/colored_box.hpp"
#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/widgets/grid_paper.hpp"
#include "SkrGui/widgets/positioned.hpp"
#include "SkrGui/widgets/sized_box.hpp"
#include "SkrGui/widgets/text.hpp"
namespace skr::gui
{
void MayBeExample()
{
    // canvas
    //   grid_paper
    //   color_picker
    //   flex
    //     colored_box
    //     colored_box
    //     colored_box
    //   text
    // auto sandbox = SkrNew<Sandbox>(
    // SNewWidget(Canvas) {
    //     SNewSlot(p.canvas_children)
    //     {
    //         p.positional.fill();
    //         p.child = SNewWidget(GridPaper){};
    //     };
    //     SNewSlot(p.canvas_children)
    //     {
    //         p.positional.fill();
    //         p.child = SNewWidget(ColorPicker){};
    //     };
    //     SNewSlot(p.canvas_children)
    //     {
    //         p.positional.anchor_LT(0, 0).sized(400, 400).pivot({ 0.5, 0 });
    //         p.child = SNewWidget(Flex)
    //         {
    //             p.align_items = AlignItems::FlexStart;
    //             p.justify_content = JustifyContent::Center;
    //             SNewSlot(p.flex_children)
    //             {
    //                 p.child = SNewWidget(SizedBox)
    //                 {
    //                     p.size = { 100, 300 };
    //                     SNewWidget(ColoredBox) { p.color = Color::SRGB("#F00"); };
    //                 };
    //             };
    //             SNewSlot(p.flex_children)
    //             {
    //                 p.child = SNewWidget(SizedBox)
    //                 {
    //                     p.size = { 100, 200 };
    //                     SNewWidget(ColoredBox) { p.color = Color::SRGB("#0F0"); };
    //                 };
    //             };
    //             SNewSlot(p.flex_children)
    //             {
    //                 p.child = SNewWidget(SizedBox)
    //                 {
    //                     p.size = { 100, 400 };
    //                     SNewWidget(ColoredBox) { p.color = Color::SRGB("#00F"); };
    //                 };
    //             };
    //         };
    //     };
    //     SNewSlot(p.canvas_children)
    //     {
    //         p.positional.anchor_LT(0.5_pct, 10_px).pivot({ 0.5, 0 });
    //         p.child = SNewWidget(Text) { p.text = u8"Hello World!"; };
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
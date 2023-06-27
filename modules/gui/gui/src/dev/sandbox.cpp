#include "SkrGui/dev/sandbox.hpp"
#include "platform/memory.h"

// !!!! TestWidgets !!!!
#include "SkrGui/widgets/stack.hpp"
#include "SkrGui/widgets/color_picker.hpp"
#include "SkrGui/widgets/colored_box.hpp"
#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/widgets/grid_paper.hpp"
#include "SkrGui/widgets/positioned.hpp"
#include "SkrGui/widgets/sized_box.hpp"
#include "SkrGui/widgets/text.hpp"
#include "SkrGui/widgets/flex_slot.hpp"

namespace skr::gui
{
Widget* _example()
{
    return SNewWidget(Stack)
    {
        SNewChild(p.children, Positioned)
        {
            p.positional.fill();
            p.child = SNewWidget(GridPaper){};
        };
        SNewChild(p.children, Positioned)
        {
            p.positional.fill();
            p.child = SNewWidget(ColorPicker){};
        };
        SNewChild(p.children, Positioned)
        {
            p.positional.anchor_LT(0, 0).sized(400, 400).pivot({ 0.5, 0 });
            p.child = SNewWidget(Flex)
            {
                p.cross_axis_alignment = ECrossAxisAlignment::Start;
                p.main_axis_alignment = EMainAxisAlignment::Center;
                SNewChild(p.children, SizedBox)
                {
                    p.size = { 100, 300 };
                    SNewWidget(ColoredBox) { p.color = Color::SRGB("#F00"); };
                };
                SNewChild(p.children, SizedBox)
                {
                    p.size = { 100, 200 };
                    SNewWidget(ColoredBox) { p.color = Color::SRGB("#0F0"); };
                };
                SNewChild(p.children, SizedBox)
                {
                    p.size = { 100, 400 };
                    SNewWidget(ColoredBox) { p.color = Color::SRGB("#00F"); };
                };
            };
        };
        SNewChild(p.children, Positioned)
        {
            p.positional.anchor_LT(0.5_pct, 10_px).pivot({ 0.5, 0 });
            p.child = SNewWidget(Text) { p.text = u8"Hello World!"; };
        };
    };
}
} // namespace skr::gui

namespace skr::gui
{
Sandbox::Sandbox(INativeDevice* device, ICanvasService* canvas_service, ITextService* text_service) SKR_NOEXCEPT
    : _device(device),
      _canvas_service(canvas_service),
      _text_service(text_service)
{
}

} // namespace skr::gui
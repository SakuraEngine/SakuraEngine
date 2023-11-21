#include "counter_state.hpp"
#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/widgets/text.hpp"
#include "SkrGui/widgets/mouse_region.hpp"
#include "SkrGui/widgets/colored_box.hpp"

namespace skr::gui
{
NotNull<Widget*> CounterState::build(NotNull<IBuildContext*> context) SKR_NOEXCEPT
{
    return SNewWidget(Flex)
    {
        p.flex_direction = EFlexDirection::Column;
        p.children += SNewWidget(Text)
        {
            p.text = skr::format(u8"count: {}", count);
        };
        for (int32_t i = 0; i < count / 10; ++i)
        {
            p.children += SNewWidget(ColoredBox)
            {
                p.color = { 0, 0, 1, 1 };
                p.child = SNewWidget(Text)
                {
                    p.text = skr::format(u8"count {}", i * 10);
                };
            };
        };

        p.children += SNewWidget(MouseRegin)
        {
            p.on_hover = [this](PointerMoveEvent* event) {
                set_state([this]() {
                    ++count;
                });
                return true;
            };
            p.child = SNewWidget_S(ColoredBox)
            {
                p.color = { 1, 0, 0, 1 };
                p.child = SNewWidget_S(Text)
                {
                    p.text = u8"increment";
                };
            };
        };
    };
}
} // namespace skr::gui
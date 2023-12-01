#include "counter_state.hpp"
#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/widgets/text.hpp"
#include "SkrGui/widgets/mouse_region.hpp"
#include "SkrGui/widgets/colored_box.hpp"
#include "SkrGui/system/input/gesture/click_gesture_recognizer.hpp"
#include "SkrGui/framework/build_context.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr::gui
{

NotNull<Widget*> CounterState::build(NotNull<IBuildContext*> context) SKR_NOEXCEPT
{
    if (!_click_gesture)
    {
        // TODO. pass input manager by setter
        _click_gesture           = SkrNew<ClickGestureRecognizer>(context->build_owner()->input_manager());
        _click_gesture->on_click = [this]() {
            set_state([this]() {
                ++count;
            });
        };
    }

    return SNewWidget(Flex)
    {
        p.flex_direction = EFlexDirection::Column;
        p.children += SNewWidget(Text)
        {
            p.text = skr::format(u8"count: {}", count);
        };
        // for (int32_t i = 0; i < count; ++i)
        // {
        //     p.children += SNewWidget(ColoredBox)
        //     {
        //         p.color = { 0, 0, 1, 1 };
        //         p.child = SNewWidget(Text)
        //         {
        //             p.text = skr::format(u8"count {}", i);
        //         };
        //     };
        // };

        p.children += SNewWidget(MouseRegin)
        {
            p.on_down = [this](PointerDownEvent* event) {
                if (event->phase != EEventRoutePhase::BubbleUp) return false;
                if (event->button == EPointerButton::Left)
                {
                    _click_gesture->add_pointer(event);
                    return true;
                }
                return false;
            };
            p.on_up = [this](PointerUpEvent* event) {
                if (event->phase != EEventRoutePhase::BubbleUp) return false;
                if (event->button == EPointerButton::Left)
                {
                    _click_gesture->handle_event_from_widget(event);
                    return true;
                }
                return false;
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
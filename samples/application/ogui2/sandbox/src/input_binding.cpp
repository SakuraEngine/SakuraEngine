#include "input_binding.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

namespace skr::gui
{
inline static void _fill_pointer_event(PointerEvent* event)
{
    int32_t cursor_x, cursor_y;
    skr_cursor_pos(&cursor_x, &cursor_y, ECursorCoordinate::CURSOR_COORDINATE_SCREEN);

    event->device_type     = EPointerDeviceType::Mouse;
    event->global_position = { (float)cursor_x, (float)cursor_y };
    event->global_delta    = { 0, 0 };
}
inline static void _bind_mouse_button(skr::input::InputSystem* system, SObjectPtr<input::InputMappingContext> ctx, Sandbox* sandbox, EPointerButton button)
{
    // translate key
    EMouseKey key;
    switch (button)
    {
        case EPointerButton::Left:
            key = EMouseKey::MOUSE_KEY_LB;
            break;
        case EPointerButton::Right:
            key = EMouseKey::MOUSE_KEY_RB;
            break;
        case EPointerButton::Middle:
            key = EMouseKey::MOUSE_KEY_MB;
            break;
        case EPointerButton::X1B:
            key = EMouseKey::MOUSE_KEY_X1B;
            break;
        case EPointerButton::X2B:
            key = EMouseKey::MOUSE_KEY_X2B;
            break;
        default:
            key = EMouseKey::MOUSE_KEY_None;
            break;
    }

    // down
    {
        auto action  = system->create_input_action(skr::input::EValueType::kBool);
        auto trigger = system->create_trigger<skr::input::InputTriggerPressed>();
        action->add_trigger(trigger);
        action->bind_event<bool>(
        [sandbox, button](const bool& down) {
            PointerDownEvent event;
            _fill_pointer_event(&event);
            event.button = button;
            sandbox->dispatch_event(&event);
        });
        auto mapping    = system->create_mapping<skr::input::InputMapping_MouseButton>(key);
        mapping->action = action;
        ctx->add_mapping(mapping);
    }

    // up
    {
        auto action  = system->create_input_action(skr::input::EValueType::kBool);
        auto trigger = system->create_trigger<skr::input::InputTriggerPressed>();
        action->add_trigger(trigger);
        action->bind_event<bool>([sandbox, button](const bool& down) {
            PointerUpEvent event;
            _fill_pointer_event(&event);
            event.button = button;
            sandbox->dispatch_event(&event);
        });
        auto mapping    = system->create_mapping<skr::input::InputMapping_MouseButton>(key);
        mapping->action = action;
        ctx->add_mapping(mapping);
    }
}

void bind_pointer_event(skr::input::InputSystem* system, SObjectPtr<input::InputMappingContext> ctx, Sandbox* sandbox)
{
    // bind buttons (down/up)
    _bind_mouse_button(system, ctx, sandbox, EPointerButton::Left);
    _bind_mouse_button(system, ctx, sandbox, EPointerButton::Right);
    _bind_mouse_button(system, ctx, sandbox, EPointerButton::Middle);
    _bind_mouse_button(system, ctx, sandbox, EPointerButton::X1B);
    _bind_mouse_button(system, ctx, sandbox, EPointerButton::X2B);

    // move
    {
        auto action  = system->create_input_action(skr::input::EValueType::kFloat2);
        auto trigger = system->create_trigger<skr::input::InputTriggerChanged>();
        action->add_trigger(trigger);
        action->bind_event<skr_float2_t>([sandbox](const skr_float2_t& delta) {
            PointerMoveEvent event;
            _fill_pointer_event(&event);
            event.global_delta = { delta.x, delta.y };
            sandbox->dispatch_event(&event);
        });
        auto mapping    = system->create_mapping<skr::input::InputMapping_MouseAxis>(EMouseAxis::MOUSE_AXIS_XY);
        mapping->action = action;
        ctx->add_mapping(mapping);
    }

    // wheel
    {
        auto action  = system->create_input_action(skr::input::EValueType::kFloat2);
        auto trigger = system->create_trigger<skr::input::InputTriggerChanged>();
        // action->add_trigger(trigger);
        action->bind_event<skr_float2_t>([sandbox](const skr_float2_t& delta) {
            PointerScrollEvent event;
            _fill_pointer_event(&event);
            event.scroll_delta = { delta.x, delta.y };
            sandbox->dispatch_event(&event);
        });
        auto mapping    = system->create_mapping<skr::input::InputMapping_MouseAxis>(EMouseAxis::MOUSE_AXIS_WHEEL_XY);
        mapping->action = action;
        ctx->add_mapping(mapping);
    }
}
} // namespace skr::gui
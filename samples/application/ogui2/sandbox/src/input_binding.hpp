#pragma once
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_trigger.hpp"
#include "SkrGui/dev/sandbox.hpp"

namespace skr::gui
{
void bind_pointer_event(skr::input::InputSystem* system, SObjectPtr<input::InputMappingContext> ctx, Sandbox* sandbox);

} // namespace skr::gui
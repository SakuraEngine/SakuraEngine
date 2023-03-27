#include "SkrInputSystem/input_trigger.hpp"
#include "SkrInputSystem/input_action.hpp"

namespace skr {
namespace input {

ETriggerState InputTriggerDown::update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT
{
    return value.get_magnitude_val() ? ETriggerState::Triggered : ETriggerState::None;
}

ETriggerState InputTriggerPressed::update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT
{
    auto triggered = value.get_magnitude_val() && !last_value.get_magnitude_val();
    last_value = value;
    return triggered ? ETriggerState::Triggered : ETriggerState::None;
}

} }
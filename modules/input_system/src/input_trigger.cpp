#include "SkrInputSystem/input_trigger.hpp"
#include "SkrInputSystem/input_action.hpp"

namespace skr {
namespace input {

InputTrigger::~InputTrigger() SKR_NOEXCEPT
{

}

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

ETriggerState InputTriggerChanged::update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT
{
    auto last = last_value.get_raw();
    auto current = value.get_raw();
    if(last.x != current.x || last.y != current.y || last.z != current.z || last.w != current.w)
    {
        last_value = value;
        return ETriggerState::Triggered;
    }
    return ETriggerState::None;
}

ETriggerState InputTriggerAlways::update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT
{
    return ETriggerState::Triggered;
}

} }
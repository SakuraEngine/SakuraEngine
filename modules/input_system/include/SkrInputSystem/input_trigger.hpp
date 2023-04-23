#pragma once
#include "SkrInputSystem/input_value.hpp"

namespace skr {
namespace input {

struct InputValueStorage;

enum class ETriggerState
{
    None,
    Ongoing,
    Triggered
};

/*
enum class ETriggerEvent
{
    None,
    Triggered,
    Started,
    Ongoing,
    Canceled,
    Completed
};
*/

struct SKR_INPUTSYSTEM_API InputTrigger : public RC
{
    virtual ~InputTrigger() SKR_NOEXCEPT;
    
    virtual ETriggerState update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT = 0;
};

struct SKR_INPUTSYSTEM_API InputTriggerDown : public InputTrigger
{
    ETriggerState update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT final;
};

struct SKR_INPUTSYSTEM_API InputTriggerPressed : public InputTrigger
{
    ETriggerState update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT final;

    InputValueStorage last_value;
    bool last_triggered = false;
};

struct SKR_INPUTSYSTEM_API InputTriggerChanged : public InputTrigger
{
    ETriggerState update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT final;

    InputValueStorage last_value;
};

struct SKR_INPUTSYSTEM_API InputTriggerAlways : public InputTrigger
{
    ETriggerState update_state(const InputValueStorage& value, float delta) SKR_NOEXCEPT final;
};

} }
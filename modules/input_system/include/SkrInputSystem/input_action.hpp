#pragma once
#include "SkrInputSystem/input_value.hpp"
#include "utils/function_ref.hpp"

namespace skr {
namespace input {

struct InputModifier;
struct InputTrigger;

template<typename ValueType>
using ActionEvent = skr::function_ref<void(const ValueType&)>;

using ActionEventId = skr_guid_t;

static const ActionEventId kEventId_Invalid = {0xbbd09231, 0xa76b, 0x4c0f, {0x83, 0x2e, 0x11, 0x7f, 0xd6, 0xac, 0x5c, 0x1b}};

struct SKR_INPUTSYSTEM_API InputAction
{
    virtual ~InputAction() SKR_NOEXCEPT;

    template<typename ValueType>
    ActionEventId bind_event(const ActionEvent<ValueType>& event, ActionEventId id = kEventId_Invalid) SKR_NOEXCEPT;

    virtual ActionEventId bind_event(const ActionEvent<InputValueStorage>& event, ActionEventId id = kEventId_Invalid) SKR_NOEXCEPT = 0;

    virtual const InputValueStorage& get_value() const SKR_NOEXCEPT = 0;

    virtual bool unbind_event(ActionEventId id) SKR_NOEXCEPT = 0;

    virtual void add_trigger(InputTrigger& trigger) SKR_NOEXCEPT = 0;

    virtual void remove_trigger(InputTrigger& trigger) SKR_NOEXCEPT = 0;

    virtual void add_modifier(InputModifier& modifier) SKR_NOEXCEPT = 0;

    virtual void remove_modifier(InputModifier& modifier) SKR_NOEXCEPT = 0;

    EValueType value_type = EValueType::kBool;

protected:
    friend struct InputMapping;
    friend struct InputSystem;
    friend struct InputSystemImpl;
    virtual void set_current_value(InputValueStorage value) SKR_NOEXCEPT = 0;
    virtual void process_modifiers(float delta) SKR_NOEXCEPT = 0;
    virtual void process_triggers(float delta) SKR_NOEXCEPT = 0;
};

} }
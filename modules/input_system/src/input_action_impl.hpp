#pragma once
#include "SkrInputSystem/input_action.hpp"
#include "SkrInputSystem/input_trigger.hpp"
#include "SkrInputSystem/input_modifier.hpp"
#include "SkrRT/platform/guid.hpp"
#include <EASTL/functional.h>
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/sptr.hpp"

namespace skr
{
namespace input
{

struct ActionEventStorage {
    eastl::function<void()> callback;
    skr_guid_t event_id = kEventId_Invalid;
};

struct SKR_INPUTSYSTEM_API InputActionImpl : public InputAction {
    InputActionImpl(EValueType type) SKR_NOEXCEPT
        : InputAction(type),
          current_value(type, skr_float4_t{ 0.f, 0.f, 0.f, 0.f })
    {
    }
    virtual ~InputActionImpl() SKR_NOEXCEPT;

    ActionEventId bind_event(const ActionEvent<InputValueStorage>& event, ActionEventId id = kEventId_Invalid) SKR_NOEXCEPT final
    {
        ActionEventStorage storage;
        storage.event_id = id;
        if (storage.event_id == kEventId_Invalid)
        {
            skr_make_guid(&storage.event_id);
        }
        storage.callback = [event, this]() {
            event(current_value);
        };
        return events.emplace_back(storage).event_id;
    }

    const InputValueStorage& get_value() const SKR_NOEXCEPT final
    {
        return current_value;
    }

    bool unbind_event(ActionEventId id) SKR_NOEXCEPT final
    {
        for (auto it = events.begin(); it != events.end(); ++it)
        {
            if (it->event_id == id)
            {
                events.erase(it);
                return true;
            }
        }
        return false;
    }

    void add_trigger(SObjectPtr<InputTrigger> trigger) SKR_NOEXCEPT
    {
        triggers.emplace_back(trigger);
    }

    void remove_trigger(SObjectPtr<InputTrigger> trigger) SKR_NOEXCEPT
    {
        for (auto it = triggers.begin(); it != triggers.end(); ++it)
        {
            if (*it == trigger)
            {
                triggers.erase(it);
                break;
            }
        }
    }

    void add_modifier(SObjectPtr<InputModifier> modifier) SKR_NOEXCEPT final
    {
        modifiers.emplace_back(modifier);
    }

    void remove_modifier(SObjectPtr<InputModifier> modifier) SKR_NOEXCEPT final
    {
        for (auto it = modifiers.begin(); it != modifiers.end(); ++it)
        {
            if (*it == modifier)
            {
                modifiers.erase(it);
                break;
            }
        }
    }

    void set_value(InputValueStorage value) SKR_NOEXCEPT final
    {
        current_value = value;
    }

    void clear_value() SKR_NOEXCEPT final
    {
        current_value = InputValueStorage(current_value.get_type(), skr_float4_t{ 0.f, 0.f, 0.f, 0.f });
    }

    void accumulate_value(InputValueStorage value) SKR_NOEXCEPT final
    {
        skr_float4_t v = current_value.get_raw();
        skr_float4_t v2 = value.get_raw();
        v.x += v2.x;
        v.y += v2.y;
        v.z += v2.z;
        v.w += v2.w;
        current_value = InputValueStorage(current_value.get_type(), v);
    }

    void process_modifiers(float delta) SKR_NOEXCEPT final
    {
        for (auto& modifier : modifiers)
        {
            current_value = modifier->modify_raw(current_value);
        }
    }

    void process_triggers(float delta) SKR_NOEXCEPT final
    {
        for (auto& trigger : triggers)
        {
            auto state = trigger->update_state(current_value, delta);
            if (state == ETriggerState::Triggered)
            {
                for (auto& event : events)
                {
                    event.callback();
                }
            }
        }
    }

protected:
    InputValueStorage current_value;
    vector<ActionEventStorage> events;
    vector<SObjectPtr<InputTrigger>> triggers;
    vector<SObjectPtr<InputModifier>> modifiers;
};

} // namespace input
} // namespace skr
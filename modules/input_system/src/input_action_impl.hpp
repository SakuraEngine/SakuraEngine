#pragma once
#define CONTAINER_LITE_IMPL
#include "SkrInputSystem/input_action.hpp"
#include "SkrInputSystem/input_trigger.hpp"
#include "SkrInputSystem/input_modifier.hpp"
#include "platform/guid.hpp"
#include <EASTL/functional.h>

namespace skr {
namespace input {

template<typename ValueType>
using ActionEventImpl = eastl::function<void(const ValueType&)>;

struct ActionEventStorage
{
    eastl::function<void()> callback;
    skr_guid_t event_id = kEventId_Invalid;
};

struct SKR_INPUTSYSTEM_API InputActionImpl : public InputAction
{
    InputActionImpl(EValueType type) SKR_NOEXCEPT
        : type(type), current_value(type, skr_float4_t{ 0.f, 0.f, 0.f, 0.f })
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
        ActionEventImpl<InputValueStorage> event_impl = event;
        storage.callback = [event_impl, this](){
            event_impl(current_value);
        };
        return events.get().emplace_back(storage).event_id;
    }

    const InputValueStorage& get_value() const SKR_NOEXCEPT final
    {
        return current_value;
    }   

    bool unbind_event(ActionEventId id) SKR_NOEXCEPT final
    {
        for (auto it = events.get().begin(); it != events.get().end(); ++it)
        {
            if (it->event_id == id)
            {
                events.get().erase(it);
                return true;
            }
        }
        return false;
    }

    void add_trigger(InputTrigger& trigger) SKR_NOEXCEPT
    {
        triggers.get().emplace_back(&trigger);
    }

    void remove_trigger(InputTrigger& trigger) SKR_NOEXCEPT
    {
        for (auto it = triggers.get().begin(); it != triggers.get().end(); ++it)
        {
            if (*it == &trigger)
            {
                triggers.get().erase(it);
                break;
            }
        }
    }

    void add_modifier(InputModifier& modifier) SKR_NOEXCEPT final
    {
        modifiers.get().emplace_back(&modifier);
    }

    void remove_modifier(InputModifier& modifier) SKR_NOEXCEPT final
    {
        for (auto it = modifiers.get().begin(); it != modifiers.get().end(); ++it)
        {
            if (*it == &modifier)
            {
                modifiers.get().erase(it);
                break;
            }
        }
    }

    void set_current_value(InputValueStorage value) SKR_NOEXCEPT final
    {
        current_value = value;
    }

    void process_modifiers(float delta) SKR_NOEXCEPT final
    {
        for (auto& modifier : modifiers.get())
        {
            current_value = modifier->modify_raw(current_value);
        }
    }

    void process_triggers(float delta) SKR_NOEXCEPT final
    {
        for (auto& trigger : triggers.get())
        {
            auto state = trigger->update_state(current_value, delta);
        }
    }

protected:
    EValueType type;
    InputValueStorage current_value;
    lite::VectorStorage<ActionEventStorage> events;
    lite::VectorStorage<InputTrigger*> triggers;
    lite::VectorStorage<InputModifier*> modifiers;
};

} }
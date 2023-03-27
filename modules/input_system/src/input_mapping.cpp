#define CONTAINER_LITE_IMPL
#include "SkrInputSystem/input_action.hpp"
#include "SkrInputSystem/input_mapping.hpp"
#include "SkrInputSystem/input_modifier.hpp"

namespace skr {
namespace input {

InputMapping::~InputMapping() SKR_NOEXCEPT
{

}

void InputMapping::add_modifier(InputModifier& modifier) SKR_NOEXCEPT
{
    modifiers.get().emplace_back(&modifier);
}

void InputMapping::remove_modifier(InputModifier& modifier) SKR_NOEXCEPT
{
    auto& modifiers_ = modifiers.get();
    for (auto it = modifiers_.begin(); it != modifiers_.end(); ++it)
    {
        if (*it == &modifier)
        {
            modifiers_.erase(it);
            break;
        }
    }
}

lite::LiteSpan<InputModifierId> InputMapping::get_modifiers() SKR_NOEXCEPT
{
    return{ modifiers.get().data(), modifiers.get().size() };
}

void InputMapping::process_modifiers(float delta) SKR_NOEXCEPT
{
    for (auto modifier : get_modifiers())
    {
        raw_value = modifier->modify_raw(raw_value);
    }
}

void InputMapping::process_actions(float delta) SKR_NOEXCEPT
{
    if (action)
    {
        action->set_current_value(raw_value);
        action->process_modifiers(delta);
        action->process_triggers(delta);
    }
}

InputMappingContext::~InputMappingContext() SKR_NOEXCEPT
{
}

lite::LiteSpan<SObjectPtr<InputMapping> const> InputMappingContext::get_mappings() const SKR_NOEXCEPT
{
    auto& mappings = mappings_.get();
    return { mappings.data(), mappings.size() };
}

SObjectPtr<InputMapping> InputMappingContext::add_mapping(SObjectPtr<InputMapping> mapping) SKR_NOEXCEPT
{
    return mappings_.get().emplace_back(mapping);
}

void InputMappingContext::remove_mapping(SObjectPtr<InputMapping> mapping) SKR_NOEXCEPT
{
    auto& mappings = mappings_.get();
    for (auto it = mappings.begin(); it != mappings.end(); ++it)
    {
        if (*it == mapping)
        {
            mappings.erase(it);
            break;
        }
    }
}

void InputMappingContext::unmap_all() SKR_NOEXCEPT
{
    mappings_.get().clear();
}


InputTypeId InputMapping_Keyboard::get_input_type() const SKR_NOEXCEPT
{
    return kInputTypeId_Keyboard;
}

void InputMapping::process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
{
    raw_value.reset();
}

void InputMapping_Keyboard::process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
{
    InputMapping::process_input_reading(layer, reading, kind);
    
    InputKeyState key_states[16];
    const auto count = layer->GetKeyState(reading, 16, key_states);
    for (uint32_t i = 0; i < count; i++)
    {
        const auto& state = key_states[i];
        if (key == state.virtual_key)
        {
            switch (action->value_type)
            {
            case EValueType::kBool:
                raw_value = InputValueStorage(true);
                break;
            case EValueType::kFloat:
                raw_value = InputValueStorage(1.f);
                break;
            case EValueType::kFloat2:
                raw_value = InputValueStorage(skr_float2_t{1.f, 0.f});
                break;
            case EValueType::kFloat3:
                raw_value = InputValueStorage(skr_float3_t{1.f, 0.f, 0.f});
                break;
            }
        }
    }
}

InputTypeId InputMapping_MouseButton::get_input_type() const SKR_NOEXCEPT
{
    return kInputTypeId_MouseButton;
}

void InputMapping_MouseButton::process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
{
    InputMapping::process_input_reading(layer, reading, kind);

    InputMouseState state = {};
    if (auto okay = layer->GetMouseState(reading, &state))
    {
        if (mouse_key & state.buttons)
        {
            switch (action->value_type)
            {
            case EValueType::kBool:
                raw_value = InputValueStorage(true);
                break;
            case EValueType::kFloat:
                raw_value = InputValueStorage(1.f);
                break;
            case EValueType::kFloat2:
                raw_value = InputValueStorage(skr_float2_t{1.f, 0.f});
                break;
            case EValueType::kFloat3:
                raw_value = InputValueStorage(skr_float3_t{1.f, 0.f, 0.f});
                break;
            }
        }
    }
}

InputTypeId InputMapping_MouseAxis::get_input_type() const SKR_NOEXCEPT
{
    return kInputTypeId_MouseAxis;
}

void InputMapping_MouseAxis::process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
{
    InputMapping::process_input_reading(layer, reading, kind);

    InputMouseState state = {};
    if (auto okay = layer->GetMouseState(reading, &state))
    {
        skr_float2_t pos_raw = { 0.f, 0.f };
        pos_raw.x = (float)state.positionX;
        pos_raw.y = (float)state.positionY;

        skr_float2_t wheel_raw = { 0.f, 0.f };
        wheel_raw.x = (float)state.wheelX;
        wheel_raw.y = (float)state.wheelY;
        
        skr_float2_t processed = { 0.f, 0.f };
        if (axis & MOUSE_AXIS_X) processed.x = pos_raw.x - old_pos.x;
        if (axis & MOUSE_AXIS_Y) processed.y = pos_raw.y - old_pos.y;
        if (axis & MOUSE_AXIS_WHEEL_X) processed.x = wheel_raw.x - old_wheel.x;
        if (axis & MOUSE_AXIS_WHEEL_Y) processed.y = wheel_raw.y - old_wheel.y;

        switch (action->value_type)
        {
        case EValueType::kBool:
            raw_value = InputValueStorage(processed.x);
            break;
        case EValueType::kFloat:
            raw_value = InputValueStorage(processed.x);
            break;
        case EValueType::kFloat2:
            raw_value = InputValueStorage(skr_float2_t{processed.x, processed.y});
            break;
        case EValueType::kFloat3:
            raw_value = InputValueStorage(skr_float3_t{processed.x, processed.y, 0.f});
            break;
        }

        old_pos = pos_raw;
        old_wheel = wheel_raw;
    }
}

} }
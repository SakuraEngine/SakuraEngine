#include "SkrBase/misc/debug.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/containers/umap.hpp"
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_modifier.hpp"
#include "./input_action_impl.hpp"

namespace skr
{
namespace input
{

struct InputSystemImpl : public InputSystem {
    InputSystemImpl() SKR_NOEXCEPT;

    void update(float delta) SKR_NOEXCEPT final;

    // create and add
    SObjectPtr<InputMappingContext> create_mapping_context() SKR_NOEXCEPT final;

    // remove but not free
    void remove_mapping_context(SObjectPtr<InputMappingContext> ctx) SKR_NOEXCEPT final;

    // add
    SObjectPtr<InputMappingContext> add_mapping_context(SObjectPtr<InputMappingContext> ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT final;

    // unmap and free all mapping contexts
    void remove_all_contexts() SKR_NOEXCEPT;

    // create
    [[nodiscard]] SObjectPtr<InputAction> create_input_action(EValueType type) SKR_NOEXCEPT final;

    skr::UMap<int32_t, SObjectPtr<InputMappingContext>> contexts;
    struct RawInput {
        RawInput() SKR_NOEXCEPT = default;
        RawInput(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
            : layer(layer),
              reading(reading)
        {
        }

        InputLayer*   layer   = nullptr;
        InputReading* reading = nullptr;
    };
    skr::UMap<EInputKind, skr::Vector<RawInput>> inputs;
    skr::Vector<SObjectPtr<InputAction>>         actions;
};

InputSystem::~InputSystem() SKR_NOEXCEPT
{
}

InputSystem* InputSystem::Create() SKR_NOEXCEPT
{
    return SkrNew<InputSystemImpl>();
}

void InputSystem::Destroy(InputSystem* system) SKR_NOEXCEPT
{
    SkrDelete(system);
}

InputSystemImpl::InputSystemImpl() SKR_NOEXCEPT
{
    inputs.try_add_default(InputKindControllerAxis);
    inputs.try_add_default(InputKindControllerButton);
    inputs.try_add_default(InputKindControllerSwitch);
    inputs.try_add_default(InputKindController);

    inputs.try_add_default(InputKindKeyboard);
    inputs.try_add_default(InputKindMouse);
    inputs.try_add_default(InputKindGamepad);

    inputs.try_add_default(InputKindTouch);
    inputs.try_add_default(InputKindMotion);

    inputs.try_add_default(InputKindArcadeStick);
    inputs.try_add_default(InputKindFlightStick);

    inputs.try_add_default(InputKindRacingWheel);
    inputs.try_add_default(InputKindUiNavigation);
}

void InputSystemImpl::update(float delta) SKR_NOEXCEPT
{
    // 1. glob inputs
    auto inputInst = skr::input::Input::GetInstance();
    for (auto& [kind, raw_inputs] : inputs)
    {
        // 1.1 clear history
        raw_inputs.clear();

        // 1.2 glob nearest readings
        InputLayer*   layer   = nullptr;
        InputReading* reading = nullptr;
        auto          r       = inputInst->GetCurrentReading(kind, nullptr, &layer, &reading);
        if (r == EInputResult::INPUT_RESULT_OK)
        {
            raw_inputs.add({ layer, reading, kind });
        }
    }

    // 2. update contexts
    for (auto& action : actions)
    {
        action->clear_value();
    }

    for (auto& [priority, context] : contexts)
    {
        auto mappings = context->get_mappings();
        for (auto& [kind, raw_inputs] : inputs)
        {
            for (auto& raw_input : raw_inputs)
            {
                for (auto mapping : mappings)
                {
                    // 2.1 update processing
                    bool dirty = mapping->process_input_reading(raw_input.layer, raw_input.reading, kind);
                    if (!dirty) continue;

                    // 2.2 update modifiers
                    mapping->process_modifiers(delta);
                    // 2.3 update actions
                    mapping->process_actions(delta);
                }
            }
        }
    }

    // 3. update actions
    for (auto& action : actions)
    {
        action->process_modifiers(delta);
        action->process_triggers(delta);
    }

    // 3. free raw inputs
    for (auto& [kind, raw_inputs] : inputs)
    {
        for (auto& raw_input : raw_inputs)
        {
            raw_input.layer->Release(raw_input.reading);
        }
    }
}

#pragma region                  InputMappingContexts
SObjectPtr<InputMappingContext> InputSystemImpl::create_mapping_context() SKR_NOEXCEPT
{
    return SObjectPtr<InputMappingContext>::Create();
}

void InputSystemImpl::remove_mapping_context(SObjectPtr<InputMappingContext> ctx) SKR_NOEXCEPT
{
    contexts.remove_value(ctx);
}

SObjectPtr<InputMappingContext> InputSystemImpl::add_mapping_context(SObjectPtr<InputMappingContext> ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT
{
    auto it = contexts.add(priority, ctx);
    if (it.already_exist())
    {
        SKR_LOG_ERROR(u8"InputSystemImpl::add_mapping_context: priority already exists");
        SKR_ASSERT(!it.already_exist());
    }
    return it.value();
}

void InputSystemImpl::remove_all_contexts() SKR_NOEXCEPT
{
    contexts.clear();
}
#pragma endregion

#pragma region          InputActions
SObjectPtr<InputAction> InputSystemImpl::create_input_action(EValueType type) SKR_NOEXCEPT
{
    auto result = SObjectPtr<InputActionImpl>::Create(type);
    actions.add(result);
    return result;
}
#pragma endregion
} // namespace input
} // namespace skr
#define CONTAINER_LITE_IMPL
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_modifier.hpp"
#include "./input_action_impl.hpp"
#include "platform/debug.h"
#include "utils/log.h"

#include <EASTL/map.h>
#include <EASTL/vector_map.h>

namespace skr {
namespace input {

struct InputSystemImpl : public InputSystem
{
    InputSystemImpl() SKR_NOEXCEPT;

    void update(float delta) SKR_NOEXCEPT final;

    // create and add
    InputMappingContext& create_mapping_context() SKR_NOEXCEPT final;
    
    // remove and free
    void free_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT final;

    // remove but not free
    void remove_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT final;

    // add
    InputMappingContext& add_mapping_context(InputMappingContext& ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT final;

    // unmap and free all mapping contexts
    void remove_all_contexts() SKR_NOEXCEPT;

    // create
    [[nodiscard]] InputAction& create_input_action(EValueType type) SKR_NOEXCEPT final;

    // free
    void free_input_action(InputAction& action) SKR_NOEXCEPT final;

    eastl::map<int32_t, InputMappingContext*> contexts;
    struct RawInput
    {
        RawInput() SKR_NOEXCEPT = default;
        RawInput(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT
            : layer(layer), reading(reading)
        {

        }

        InputLayer* layer = nullptr;
        InputReading* reading = nullptr;
    };
    eastl::vector_map<EInputKind, eastl::vector<RawInput>> inputs;
};

InputSystem::~InputSystem() SKR_NOEXCEPT
{

}

InputSystem* InputSystem::Create() SKR_NOEXCEPT
{
    return SkrNew<InputSystemImpl>();
}

void InputSystem::Destroy() SKR_NOEXCEPT
{
    SkrDelete(this);
}

InputSystemImpl::InputSystemImpl() SKR_NOEXCEPT
{
    inputs[InputKindControllerAxis] = {};
    inputs[InputKindControllerButton] = {};
    inputs[InputKindControllerSwitch] = {};
    inputs[InputKindController] = {};

    inputs[InputKindKeyboard] = {};
    inputs[InputKindMouse] = {};
    inputs[InputKindGamepad] = {};

    inputs[InputKindTouch] = {};
    inputs[InputKindMotion] = {};

    inputs[InputKindArcadeStick] = {};
    inputs[InputKindFlightStick] = {};

    inputs[InputKindRacingWheel] = {};
    inputs[InputKindUiNavigation] = {};
}

void InputSystemImpl::update(float delta) SKR_NOEXCEPT
{
    // 1. glob inputs
    auto inputInst = skr::input::Input::GetInstance();
    for (auto [kind, input] : inputs)
    {
        // 1.1 clear history
        input.clear();

        // 1.2 glob nearest readings
        InputLayer* layer = nullptr;
        InputReading* reading = nullptr;
        auto r = inputInst->GetCurrentReading(kind, nullptr, &layer, &reading);
        if (r == EInputResult::INPUT_RESULT_OK)
        {
            input.emplace_back(layer, reading, kind);
        }
    }

    // 2. update contexts
    for (auto [priority, context] : contexts)
    {
        auto mappings = context->get_mappings();
        for (auto [kind, raw_inputs] : inputs)
        {
            for (auto raw_input : raw_inputs)
            {
                for (auto mapping : mappings)
                {
                    // 2.1 update processing
                    mapping->process_input_reading(raw_input.layer, raw_input.reading, kind);
                    // 2.2 update modifiers
                    mapping->process_modifiers(delta);
                    // 2.3 update actions
                    mapping->process_actions(delta);
                }
            }
        }
    }
}

#pragma region InputMappingContexts
InputMappingContext& InputSystemImpl::create_mapping_context() SKR_NOEXCEPT
{
    auto result = SkrNew<InputMappingContext>();
    return *result;
}

void InputSystemImpl::free_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT
{
    SkrDelete(&ctx);
}

void InputSystemImpl::remove_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT
{
    for (auto it = contexts.begin(); it != contexts.end(); ++it)
    {
        if (it->second == &ctx)
        {
            contexts.erase(it);
            break;
        }
    }
}

InputMappingContext& InputSystemImpl::add_mapping_context(InputMappingContext& ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT
{
    auto it = contexts.emplace(priority, &ctx);
    if (!it.second)
    {
        SKR_LOG_ERROR("InputSystemImpl::add_mapping_context: priority already exists");
        SKR_ASSERT(it.second);
    }
    return it.second ? *it.first->second : ctx;
}

void InputSystemImpl::remove_all_contexts() SKR_NOEXCEPT
{
    for (auto it = contexts.begin(); it != contexts.end(); ++it)
    {
        SkrDelete(it->second);
    }
    contexts.clear();
}
#pragma endregion

#pragma region InputActions
InputAction& InputSystemImpl::create_input_action(EValueType type) SKR_NOEXCEPT 
{
    auto result = SkrNew<InputActionImpl>(type);
    return *result;
}

void InputSystemImpl::free_input_action(InputAction& action) SKR_NOEXCEPT 
{
    SkrDelete(&action);
}
#pragma endregion
} }
#pragma once
#include "SkrInputSystem/input_mapping.hpp"
#include "SkrInputSystem/input_action.hpp"
#include "SkrInputSystem/input_trigger.hpp"

namespace skr {
namespace input {

struct InputContextOptions
{
    uint32_t nothing_and_useless = 0;
};

struct SKR_INPUTSYSTEM_API InputSystem
{
    virtual ~InputSystem() SKR_NOEXCEPT;

    [[nodiscard]] static InputSystem* Create() SKR_NOEXCEPT;
    static void Destroy(InputSystem* system) SKR_NOEXCEPT;

    virtual void update(float delta) SKR_NOEXCEPT = 0;

#pragma region InputMappingContexts
    // create
    [[nodiscard]] virtual SObjectPtr<InputMappingContext> create_mapping_context() SKR_NOEXCEPT = 0;

    // remove
    virtual void remove_mapping_context(SObjectPtr<InputMappingContext> ctx) SKR_NOEXCEPT = 0;

    // add
    virtual SObjectPtr<InputMappingContext> add_mapping_context(SObjectPtr<InputMappingContext> ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT = 0;

    // unmap and free all mapping contexts
    virtual void remove_all_contexts() SKR_NOEXCEPT = 0;
#pragma endregion

#pragma region InputMapping
    template<typename T, typename...Args>
    SObjectPtr<T> create_mapping(Args&&...args) SKR_NOEXCEPT
    {
        return SObjectPtr<T>::Create(std::forward<Args>(args)...);
    }
#pragma endregion

#pragma region InputTrigger
    template<typename T, typename...Args>
    SObjectPtr<T> create_trigger(Args&&...args) SKR_NOEXCEPT
    {
        return SObjectPtr<T>::Create(std::forward<Args>(args)...);
    }
#pragma endregion

#pragma region InputActions
    // create
    [[nodiscard]] virtual SObjectPtr<InputAction> create_input_action(EValueType type) SKR_NOEXCEPT = 0;
#pragma endregion
};

} }

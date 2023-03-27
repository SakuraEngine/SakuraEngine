#pragma once
#include "SkrInputSystem/input_mapping.hpp"
#include "SkrInputSystem/input_action.hpp"

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
    void Destroy() SKR_NOEXCEPT;

    virtual void update(float delta) SKR_NOEXCEPT = 0;

#pragma region InputMappingContexts
    // create
    [[nodiscard]] virtual InputMappingContext& create_mapping_context() SKR_NOEXCEPT = 0;
    
    // remove
    virtual void free_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT = 0;

    // remove
    virtual void remove_mapping_context(InputMappingContext& ctx) SKR_NOEXCEPT = 0;

    // add
    virtual InputMappingContext& add_mapping_context(InputMappingContext& ctx, int32_t priority, const InputContextOptions& opts) SKR_NOEXCEPT = 0;

    // unmap and free all mapping contexts
    virtual void remove_all_contexts() SKR_NOEXCEPT = 0;
#pragma endregion

#pragma region InputActions
    // create
    [[nodiscard]] virtual InputAction& create_input_action(EValueType type) SKR_NOEXCEPT = 0;

    // free
    virtual void free_input_action(InputAction& action) SKR_NOEXCEPT = 0;
#pragma endregion
};

} }

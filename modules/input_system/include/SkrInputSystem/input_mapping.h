#pragma once
#include "SkrInputSystem/module.configure.h"
#include "platform/input.h"
#include "containers/lite.hpp"

namespace skr {
namespace input {

typedef struct skr_guid_t InputTypeId;
typedef struct InputAction* InputActionId;
typedef struct InputModifier* InputModifierId;

static const skr_guid_t kInputTypeId_Keyboard = {0x2229d021, 0x97a0, 0x4a11, {0x97, 0x26, 0xc4, 0x7b, 0x26, 0xa4, 0x61, 0x79}};
static const skr_guid_t kInputTypeId_Mouse = {0x8578d530, 0x909d, 0x44f3, {0xab, 0x6a, 0x83, 0xd6, 0x57, 0xb5, 0xbc, 0x8e}};

struct SKR_INPUTSYSTEM_API InputMapping
{
    virtual ~InputMapping() SKR_NOEXCEPT;

    virtual InputTypeId get_type() const SKR_NOEXCEPT = 0;

    InputActionId action = nullptr;
    bool runtime_mappable = false;
    lite::VectorStorage<InputModifierId> modifiers;
};

struct SKR_INPUTSYSTEM_API InputMapping_Keyboard : public InputMapping
{
    InputTypeId get_type() const SKR_NOEXCEPT override;

    EKeyCode key;
};

struct SKR_INPUTSYSTEM_API InputMapping_Mouse : public InputMapping
{
    InputTypeId get_type() const SKR_NOEXCEPT override;

    EMouseKey mouse_key;
};

} }

#pragma once
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/sptr.hpp"
#include "SkrRT/containers/lite.hpp"
#include "SkrInput/input.h"
#include "SkrInputSystem/input_value.hpp"

namespace skr {
namespace input {

struct InputLayer;
struct InputReading;

typedef struct skr_guid_t InputTypeId;
typedef struct InputAction* InputActionId;
typedef struct InputModifier* InputModifierId;

static const skr_guid_t kInputTypeId_Keyboard = {0x2229d021, 0x97a0, 0x4a11, {0x97, 0x26, 0xc4, 0x7b, 0x26, 0xa4, 0x61, 0x79}};
static const skr_guid_t kInputTypeId_MouseButton = {0x8578d530, 0x909d, 0x44f3, {0xab, 0x6a, 0x83, 0xd6, 0x57, 0xb5, 0xbc, 0x8e}};
static const skr_guid_t kInputTypeId_MouseAxis = {0xe9135be4, 0x3543, 0x4995, {0xa4, 0xb2, 0x73, 0x63, 0xea, 0x2f, 0x22, 0x03}};

struct SKR_INPUTSYSTEM_API InputMapping : public RC
{
    InputMapping() SKR_NOEXCEPT = default;
    virtual ~InputMapping() SKR_NOEXCEPT;

    virtual InputTypeId get_input_type() const SKR_NOEXCEPT = 0;
    
    virtual void add_modifier(InputModifier& modifier) SKR_NOEXCEPT;

    virtual void remove_modifier(InputModifier& modifier) SKR_NOEXCEPT;

    SObjectPtr<InputAction> action = nullptr;

protected:
    friend struct InputSystem;
    friend struct InputSystemImpl;
    virtual bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT;
    virtual lite::LiteSpan<InputModifierId> get_modifiers() SKR_NOEXCEPT;

    virtual void process_modifiers(float delta) SKR_NOEXCEPT;
    virtual void process_actions(float delta) SKR_NOEXCEPT;

    bool runtime_mappable = false;
    //TODO: modifier ownership?
    vector<InputModifierId> modifiers;
    InputValueStorage raw_value;
};

struct SKR_INPUTSYSTEM_API InputMappingContext : public RC
{
public:
    virtual ~InputMappingContext() SKR_NOEXCEPT;

    // add
    SObjectPtr<InputMapping> add_mapping(SObjectPtr<InputMapping> mapping) SKR_NOEXCEPT;

    // remove but not free
    void remove_mapping(SObjectPtr<InputMapping> mapping) SKR_NOEXCEPT;

    // get all mappings
    lite::LiteSpan<SObjectPtr<InputMapping> const> get_mappings() const SKR_NOEXCEPT;

    // unmap and free all mappings
    void unmap_all() SKR_NOEXCEPT;

protected:
    vector<SObjectPtr<InputMapping>> mappings_;
};

struct SKR_INPUTSYSTEM_API InputMapping_Keyboard : public InputMapping
{
    inline InputMapping_Keyboard(EKeyCode key)
        : InputMapping(), key(key)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;

    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;
    
    const EKeyCode key;
};

struct SKR_INPUTSYSTEM_API InputMapping_MouseButton : public InputMapping
{
    inline InputMapping_MouseButton(EMouseKey key)
        : InputMapping(), mouse_key(key)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;
    
    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;

    const EMouseKey mouse_key;
};

struct SKR_INPUTSYSTEM_API InputMapping_MouseAxis : public InputMapping
{
    inline InputMapping_MouseAxis(EMouseAxis axis)
        : InputMapping(), axis(axis)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;
    
    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;

    const EMouseAxis axis;
    skr_float2_t old_pos = {0.f, 0.f};
    skr_float2_t old_wheel = {0.f, 0.f};
};
} }

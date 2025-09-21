#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrCore/memory/rc.hpp"
#include "SkrContainers/span.hpp"
#include "SkrSystem/advanced_input.h"
#include "SkrInputSystem/input_value.hpp"
#include "SkrContainers/span.hpp"

namespace skr
{
namespace input
{

struct InputLayer;
struct InputReading;

typedef GUID            InputTypeId;
typedef struct InputAction*   InputActionId;
typedef struct InputModifier* InputModifierId;

static const GUID kInputTypeId_Keyboard    = u8"03815561-7d18-4048-98d5-f2a51cf50484"_guid;
static const GUID kInputTypeId_MouseButton = u8"71ade68a-4ff3-4d4f-b205-0b03531483ed"_guid;
static const GUID kInputTypeId_MouseAxis   = u8"5e0c24fd-02b4-47fa-af50-e50c62c1d1da"_guid;

struct SKR_INPUT_SYSTEM_API InputMapping {
    SKR_RC_IMPL();

    InputMapping() SKR_NOEXCEPT = default;
    virtual ~InputMapping() SKR_NOEXCEPT;

    virtual InputTypeId get_input_type() const SKR_NOEXCEPT = 0;

    virtual void add_modifier(InputModifier& modifier) SKR_NOEXCEPT;

    virtual void remove_modifier(InputModifier& modifier) SKR_NOEXCEPT;

    RC<InputAction> action = nullptr;

protected:
    friend struct InputSystem;
    friend struct InputSystemImpl;
    virtual bool                       process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT;
    virtual skr::Span<InputModifierId> get_modifiers() SKR_NOEXCEPT;

    virtual void process_modifiers(float delta) SKR_NOEXCEPT;
    virtual void process_actions(float delta) SKR_NOEXCEPT;

    bool runtime_mappable = false;
    // TODO: modifier ownership?
    skr::Vector<InputModifierId> modifiers;
    InputValueStorage            raw_value;
};

struct SKR_INPUT_SYSTEM_API InputMappingContext {
    SKR_RC_IMPL();

public:
    virtual ~InputMappingContext() SKR_NOEXCEPT;

    // add
    RC<InputMapping> add_mapping(RC<InputMapping> mapping) SKR_NOEXCEPT;

    // remove but not free
    void remove_mapping(RC<InputMapping> mapping) SKR_NOEXCEPT;

    // get all mappings
    Span<RC<InputMapping> const> get_mappings() const SKR_NOEXCEPT;

    // unmap and free all mappings
    void unmap_all() SKR_NOEXCEPT;

protected:
    skr::Vector<RC<InputMapping>> mappings_;
};

struct SKR_INPUT_SYSTEM_API InputMapping_Keyboard : public InputMapping {
    inline InputMapping_Keyboard(EKeyCode key)
        : InputMapping()
        , key(key)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;

    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;

    const EKeyCode key;
};

struct SKR_INPUT_SYSTEM_API InputMapping_MouseButton : public InputMapping {
    inline InputMapping_MouseButton(EMouseKey key)
        : InputMapping()
        , mouse_key(key)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;

    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;

    const EMouseKey mouse_key;
};

struct SKR_INPUT_SYSTEM_API InputMapping_MouseAxis : public InputMapping {
    inline InputMapping_MouseAxis(EMouseAxis axis)
        : InputMapping()
        , axis(axis)
    {
    }

    InputTypeId get_input_type() const SKR_NOEXCEPT override;

    bool process_input_reading(InputLayer* layer, InputReading* reading, EInputKind kind) SKR_NOEXCEPT final;

    const EMouseAxis axis;
    float2     old_pos   = { 0.f, 0.f };
    float2     old_wheel = { 0.f, 0.f };
};
} // namespace input
} // namespace skr

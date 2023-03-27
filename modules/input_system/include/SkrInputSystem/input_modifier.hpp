#pragma once
#include "SkrInputSystem/input_value.hpp"

namespace skr {
namespace input {

struct SKR_INPUTSYSTEM_API InputModifier
{
    virtual InputValueStorage modify_raw(const InputValueStorage& raw) SKR_NOEXCEPT = 0;
};

} }
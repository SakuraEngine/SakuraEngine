#pragma once
#include "SkrInput/module.configure.h"
#include "platform/configure.h"

namespace skr {
namespace input {

struct SKR_INPUT_API InputReading
{
    virtual ~InputReading() SKR_NOEXCEPT;
};

} }
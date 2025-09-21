#pragma once
#include "SkrContainers/string.hpp"
#include <SkrBase/config.h>
#include <SkrBase/meta.h>
#include "V8Playground/debug.generated.h"

namespace skr
{
struct [[sscript_visible, sattr(
    guid = "98fb6fdf-7223-4baf-90b0-46e746af88d2"
)]] Debug
{

    [[sscript_visible]]
    static void info(StringView message);

    [[sscript_visible]]
    static void warn(StringView message);

    [[sscript_visible]]
    static void error(StringView message);

    [[sscript_visible]]
    static void exit(int32_t exit_code);

    [[sscript_visible]]
    static void wait(int32_t ms);

private:
    bool _shit;
};
} // namespace skr
#pragma once
#include "SkrRT/platform/guid.hpp" // IWYU pragma: export
#include "SkrRT/platform/crash.h" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrRT/containers/sptr.hpp" // IWYU pragma: export

#include "SkrTestFramework/framework.hpp"

struct SPTRTestsBase
{
    static struct ProcInitializer
    {
        ProcInitializer();
        ~ProcInitializer();
    } init;
};
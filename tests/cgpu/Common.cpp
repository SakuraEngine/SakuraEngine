#include "SkrRT/misc/log.h"
#include "SkrRT/platform/crash.h"
#include "SkrTestFramework/framework.hpp"
#include <catch2/generators/catch_generators.hpp>

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

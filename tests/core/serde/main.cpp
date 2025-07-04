#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer {
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
    }
    ~ProcInitializer()
    {
        // ::skr_finalize_crash_handler();
    }
} init;

#include "binary.cpp"
#include "json.cpp"


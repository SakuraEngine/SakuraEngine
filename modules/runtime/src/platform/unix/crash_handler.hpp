#pragma once
#include "SkrRT/platform/crash.h"

struct SUnixCrashHandler : public SCrashHandler
{
    SUnixCrashHandler() SKR_NOEXCEPT;
    virtual ~SUnixCrashHandler() SKR_NOEXCEPT;
};
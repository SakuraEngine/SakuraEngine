#pragma once
#include "SkrCore/crash.h"

struct SUnixCrashHandler : public SCrashHandler
{
    SUnixCrashHandler() SKR_NOEXCEPT;
    virtual ~SUnixCrashHandler() SKR_NOEXCEPT;
};
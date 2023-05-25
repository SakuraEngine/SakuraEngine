#pragma once
#include "platform/thread.h"
#include "platform/atomic.h"

namespace skr
{
struct RUNTIME_API ServiceThread
{

protected:
    SThreadHandle thread;
    SThreadID id;
};
}
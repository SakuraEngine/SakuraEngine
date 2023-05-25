#pragma once
#include "async/result.hpp"
#include "platform/thread.h"
#include "platform/atomic.h"

#include "containers/string.hpp"

namespace skr
{

struct NamedThreadDesc
{
    const char8_t *name;
};
using NamedThreadPriority = SThreadPriority;
using NamedThreadResult = AsyncResult;

static constexpr NamedThreadResult NAMED_THREAD_RESULT_OK = ASYNC_RESULT_OK;
static constexpr NamedThreadResult NAMED_THREAD_RESULT_ERROR_THREAD_ALREADY_STARTES = ASYNC_RESULT_ERROR_THREAD_ALREADY_STARTES;

struct NamedThreadFunction
{
    virtual ~NamedThreadFunction() SKR_NOEXCEPT;
    virtual NamedThreadResult run() SKR_NOEXCEPT = 0;
};

struct RUNTIME_STATIC_API NamedThread
{
public:
    NamedThread() SKR_NOEXCEPT;
    NamedThread(NamedThreadPriority priority, uint32_t stackSize, const NamedThreadDesc *desc = nullptr) SKR_NOEXCEPT;
    virtual ~NamedThread() SKR_NOEXCEPT;
	
    // start the thread.
    // @retval ASYNC_RESULT_OK if success
    NamedThreadResult start(NamedThreadFunction *pFunc) SKR_NOEXCEPT;

    // wait for thread completion.
    // @retval ASYNC_RESULT_OK if success
    NamedThreadResult join() SKR_NOEXCEPT;

    // check if thread is alive.
    bool is_alive() const SKR_NOEXCEPT;

    // get thread id.
    SThreadID get_id() const SKR_NOEXCEPT;

    // change thread priority.
    SThreadPriority change_priority(SThreadPriority priority) SKR_NOEXCEPT;

    // get thread function object.
    NamedThreadFunction* get_function() const SKR_NOEXCEPT;

    // initlaize thread.
    // @retval ASYNC_RESULT_OK if success
    NamedThreadResult initialize(int32_t priority, uint32_t stack_size, const NamedThreadDesc *pdesc = nullptr) SKR_NOEXCEPT;
    
    // finalize thread.
    // @retval ASYNC_RESULT_OK if success
    NamedThreadResult finalize() SKR_NOEXCEPT;

private:
    skr::string tname;
    NamedThreadDesc desc = {};

    static void threadFunc(void* args);
    SThreadDesc tDesc;
    SThreadID tID;
    SThreadHandle tHandle;
    SAtomic32 started = false;
    SAtomic32 alive = false;
    NamedThreadFunction* func = nullptr;
};

}
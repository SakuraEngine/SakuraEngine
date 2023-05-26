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
    int32_t priority = SKR_THREAD_NORMAL;
    uint32_t stack_size = 16 * 1024;
};

struct NamedThreadFunction
{
    virtual ~NamedThreadFunction() SKR_NOEXCEPT;
    virtual AsyncResult run() SKR_NOEXCEPT = 0;
};

struct RUNTIME_STATIC_API NamedThread
{
public:
    NamedThread() SKR_NOEXCEPT;
    virtual ~NamedThread() SKR_NOEXCEPT;
	
    // start the thread.
    // @retval ASYNC_RESULT_OK if success
    AsyncResult start(NamedThreadFunction* pFunc) SKR_NOEXCEPT;

    // wait for thread completion.
    // @retval ASYNC_RESULT_OK if success
    AsyncResult join() SKR_NOEXCEPT;

    // check if thread is alive.
    bool is_alive() const SKR_NOEXCEPT;

    // check if thread is alive.
    bool has_started() const SKR_NOEXCEPT;

    // get thread id.
    SThreadID get_id() const SKR_NOEXCEPT;

    // change thread priority.
    SThreadPriority change_priority(SThreadPriority priority) SKR_NOEXCEPT;

    // get thread function object.
    NamedThreadFunction* get_function() const SKR_NOEXCEPT;

    // initlaize thread.
    // @retval ASYNC_RESULT_OK if success
    AsyncResult initialize(const NamedThreadDesc& desc = {}) SKR_NOEXCEPT;
    
    // finalize thread.
    // @retval ASYNC_RESULT_OK if success
    AsyncResult finalize() SKR_NOEXCEPT;

private:
    skr::string tname;
    NamedThreadDesc desc = {};

    static void threadFunc(void* args);
    SThreadDesc tDesc;
    SThreadID tID;
    SThreadHandle tHandle;
    SAtomic32 started = false;
    SAtomic32 alive = false;
    SAtomic32 priority = false;
    NamedThreadFunction* func = nullptr;
};

}
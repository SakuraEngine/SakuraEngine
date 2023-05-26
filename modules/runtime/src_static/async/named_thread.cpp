#include "async/named_thread.hpp"

namespace skr
{
NamedThreadFunction::~NamedThreadFunction() SKR_NOEXCEPT
{

}

NamedThread::NamedThread() SKR_NOEXCEPT
    : started(false), alive(false), func(nullptr)
{

}

NamedThread::~NamedThread() SKR_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&started)) 
    {
        skr_destroy_thread(tHandle);
        skr_atomic32_store_release(&started, false);
    }
}

void NamedThread::threadFunc(void* args)
{
    NamedThread* pSelf = (NamedThread*)args;
    pSelf->tID = skr_current_thread_id();
    
    skr_atomic32_store_release(&pSelf->started, true);
    skr_atomic32_store_release(&pSelf->alive, true);

    pSelf->func->run();
    
    skr_atomic32_store_release(&pSelf->alive, false);
}

AsyncResult NamedThread::start(NamedThreadFunction* pFunc) SKR_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&started)) 
    {
        return ASYNC_RESULT_ERROR_THREAD_ALREADY_STARTES;
    }
    
    this->func = pFunc;

    tDesc.pFunc = &threadFunc;
    tDesc.pData = this;
    skr_init_thread(&tDesc, &tHandle);
    skr_thread_set_name(tHandle, tname.u8_str());
    
    const auto P = (SThreadPriority)skr_atomic32_load_acquire(&priority);
    skr_thread_set_priority(tHandle, P);

    // wait started
    while (!skr_atomic32_load_acquire(&started)) {}

    return ASYNC_RESULT_OK;
}

AsyncResult NamedThread::join() SKR_NOEXCEPT
{
    skr_join_thread(tHandle);
    return ASYNC_RESULT_OK;
}

bool NamedThread::is_alive() const SKR_NOEXCEPT
{
    return skr_atomic32_load_acquire(&alive);
}

SThreadID NamedThread::get_id() const SKR_NOEXCEPT
{
    return tID;
}

SThreadPriority NamedThread::change_priority(SThreadPriority priority) SKR_NOEXCEPT
{
    skr_atomic32_store_relaxed(&priority, priority);
    return skr_thread_set_priority(tHandle, priority);
}

NamedThreadFunction* NamedThread::get_function() const SKR_NOEXCEPT
{
    return func;
}

AsyncResult NamedThread::initialize(const NamedThreadDesc *pdesc) SKR_NOEXCEPT
{
    tname = skr::string::from_utf8(desc.name ? desc.name : u8"unnamed");
    if (pdesc)
    {
        desc = *pdesc;
    }
    skr_atomic32_store_relaxed(&priority, priority);
    return ASYNC_RESULT_OK;
}

AsyncResult NamedThread::finalize() SKR_NOEXCEPT
{
    return ASYNC_RESULT_OK;
}

}
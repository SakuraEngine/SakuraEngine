#include "async/named_thread.hpp"

namespace skr
{
NamedThreadFunction::~NamedThreadFunction() SKR_NOEXCEPT
{

}

NamedThread::NamedThread() SKR_NOEXCEPT
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

NamedThread::NamedThread(NamedThreadPriority priority, uint32_t stack_size, const NamedThreadDesc *desc) SKR_NOEXCEPT
    : started(false), alive(false), func(nullptr)
{
    initialize(priority, stack_size, desc);
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

NamedThreadResult NamedThread::start(NamedThreadFunction* pFunc) SKR_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&started)) 
        return NAMED_THREAD_RESULT_ERROR_THREAD_ALREADY_STARTES;
    
    this->func = pFunc;

    tDesc.pFunc = &threadFunc;
    tDesc.pData = this;
    skr_init_thread(&tDesc, &tHandle);
    skr_thread_set_name(tHandle, tname.u8_str());
    
    // wait started
    while (!skr_atomic32_load_acquire(&started)) {}

    return NAMED_THREAD_RESULT_OK;
}

NamedThreadResult NamedThread::join() SKR_NOEXCEPT
{
    skr_join_thread(tHandle);
    return NAMED_THREAD_RESULT_OK;
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
    return skr_thread_set_priority(tHandle, priority);
}

NamedThreadFunction* NamedThread::get_function() const SKR_NOEXCEPT
{
    return func;
}

NamedThreadResult NamedThread::initialize(int32_t p, uint32_t stackSize, const NamedThreadDesc *pdesc) SKR_NOEXCEPT
{
    tname = skr::string::from_utf8(desc.name ? desc.name : u8"unnamed");
    if (pdesc)
    {
        desc = *pdesc;
    }
    return NAMED_THREAD_RESULT_OK;
}

NamedThreadResult NamedThread::finalize() SKR_NOEXCEPT
{
    return NAMED_THREAD_RESULT_OK;
}

}
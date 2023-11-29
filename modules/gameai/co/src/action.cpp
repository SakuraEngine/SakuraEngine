#include "SkrGameAICo/action.hpp"

#if __cpp_impl_coroutine
namespace skr
{
namespace gameai
{
    void cocontext_t::FrameAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        ctx->pendingFrameWaiters.push_back(handle);
    }

    void cocontext_t::TimerAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        ctx->pendingTimeWaiters.push({handle, time});
    }

    void cocontext_t::interupt_single(std::coroutine_handle<> handle) noexcept
    {
        pendingFrameWaiters.erase(std::remove(pendingFrameWaiters.begin(), pendingFrameWaiters.end(), handle), pendingFrameWaiters.end());
        decltype(pendingTimeWaiters) tw;
        while(!pendingTimeWaiters.empty())
        {
            auto& waiter = pendingTimeWaiters.top();
            if(waiter.handle != handle)
            {
                tw.push(waiter);
            }
            pendingTimeWaiters.pop();
        }
        pendingTimeWaiters = std::move(tw);
    }

    void cocontext_t::interupt_tree(std::coroutine_handle<> handle) noexcept
    {
        auto h = std::coroutine_handle<coaction_promise_base_t>::from_address(handle.address());
        for (auto& child : h.promise().children)
        {
            interupt_tree(child);
        }
        if(h.promise().children.empty())
            interupt_single(handle);
    }

    void cocontext_t::update(double time, bool nextFrame) noexcept
    {
        deltaTime = time - this->time;
        this->time = time;
        while(!pendingTimeWaiters.empty())
        {
            auto& waiter = pendingTimeWaiters.top();
            if(waiter.time > time)
                break;
            pendingTimeWaiters.pop();
            waiter.handle.resume();
        }
        if(nextFrame)
        {
            auto frameWaiters = std::move(pendingFrameWaiters);
            for(auto& waiter : frameWaiters)
            {
                waiter.resume();
            }
        }
    }
}
}
#endif
#include "SkrRT/platform/crash.h"
#include "SkrRT/misc/log.h"

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_DEBUG);
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

#if __cpp_impl_coroutine
#include "SkrGameAICo/action.hpp"
#include <algorithm>
using namespace skr::gameai;
struct npc
{
    coaction<> move(cocontext_t& ctx, double target)
    {
        //set animation to idle
        while(current != target)
        {
            //set animation to walk
            double deltaTime = co_await ctx.next_frame();
            current += std::min(target - current, 1.0 * deltaTime);
            
            SKR_LOG_WARN(u8"TIME:%lf POS:%lf", ctx.time, current);
        }
        co_return succeed;
    }

    coaction<> wait(cocontext_t& ctx, double time)
    {
        co_await ctx.wait(time);
        co_return succeed;
    }

    coaction<> test(cocontext_t& ctx)
    {
        co_await move(ctx, 10);
        co_await wait(ctx, 20);
        auto [a, b] = co_await race(ctx, move(ctx, 30), wait(ctx, 5));
        if(a == coaction_state::succeed)
        {
            SKR_LOG_WARN(u8"MOVE SUCCEED");
        }
        else
        {
            SKR_LOG_WARN(u8"MOVE FAILED");
        }
        co_await wait(ctx, 10);
        co_await move(ctx, 20);
        co_return succeed;
    }

    double current = 0;
};

int main()
{
    npc n;
    cocontext_t ctx;
    auto root = n.test(ctx);
    root.resume();
    for(int i=0; i<100; ++i)
    {
        ctx.update(i * 1, true);
    }
}
#else

int main()
{
    SKR_LOG_WARN(u8"coroutine not supported");
}
#endif
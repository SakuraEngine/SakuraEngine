#include "ftl/task_scheduler.h"
#include "ftl/task_counter.h"

namespace skr
{
template <class F, class Iter>
void parallel_for(ftl::TaskScheduler* scheduler, Iter begin, Iter end, size_t batch, F f)
{
    SKR_ASSERT(scheduler != nullptr);
    struct Payload {
        F* f;
        Iter begin;
        Iter end;
    };
    typename std::iterator_traits<Iter>::difference_type n = std::distance(begin, end);
    size_t batchCount = (n / batch) + 1;
    auto payloads = (Payload*)sakura_malloc(batchCount * sizeof(Payload));
    ftl::Task* tasks = (ftl::Task*)sakura_malloc(batchCount * sizeof(ftl::Task));
    auto body = +[](ftl::TaskScheduler* task, void* data) {
        auto payload = (Payload*)data;
        (*payload->f)(payload->begin, payload->end);
    };
    for (size_t i = 0; i < batchCount; ++i)
    {
        payloads[i].f = &f;
        payloads[i].begin = begin;
        auto toAdvance = std::min((size_t)n, batch);
        std::advance(begin, toAdvance);
        n -= toAdvance;
        payloads[i].end = begin;
        tasks[i] = { body, &payloads[i] };
    }
    ftl::TaskCounter counter(scheduler);
    scheduler->AddTasks(static_cast<uint32_t>(batchCount), tasks, ftl::TaskPriority::Normal, &counter);
    sakura_free(tasks);
    scheduler->WaitForCounter(&counter);
    sakura_free(payloads);
}
} // namespace skr
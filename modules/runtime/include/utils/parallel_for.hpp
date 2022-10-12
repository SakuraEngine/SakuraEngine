#include "task/task.hpp"
#include "EASTL/vector.h"

namespace skr
{
template <class F, class Iter>
void parallel_for(Iter begin, Iter end, size_t batch, F f)
{
    typename std::iterator_traits<Iter>::difference_type n = std::distance(begin, end);
    size_t batchCount = (n / batch) + 1;
    task::counter_t counter;
    counter.add(batchCount);
    for (size_t i = 0; i < batchCount; ++i)
    {
        auto toAdvance = std::min((size_t)n, batch);
        auto l = begin;
        auto r = std::advance(begin, toAdvance);
        n -= toAdvance;
        skr::task::schedule([counter, f, l, r] () mutable
            {
                SKR_DEFER({counter.decrement();});
                f(l, r);
            });
        begin = r;
    }
    counter.wait(true);
}
} // namespace skr
#include "SkrGui/framework/timer_manager.hpp"

namespace skr::gui
{
// update
void TimerManager::update(float time_stamp)
{
    _cur_time_stamp = time_stamp;
    for (auto iter = _timers.iter(); iter.has_next();)
    {
        auto&       data              = iter.ref();
        auto&       timer             = data.timer;
        const float local_time_stamp  = _cur_time_stamp - data.initial_time_stamp;
        const float repeat_time_stamp = local_time_stamp - timer.first_signal_scape;
        bool        need_remove_timer = false;

        // signal timer
        if (repeat_time_stamp >= 0)
        {
            if (data.last_signal_time_stamp < 0)
            {
                data.last_signal_time_stamp = timer.first_signal_scape;
            }

            if (timer.signal_scape == 0)
            {
                // update data
                ++data.signal_count;
                data.last_signal_time_stamp = local_time_stamp;

                // signal
                need_remove_timer = timer.callback({ data.last_signal_time_stamp, data.signal_count });
            }
            else
            {
                while ((timer.repeat_count == 0 || timer.repeat_count > data.signal_count) &&
                       data.last_signal_time_stamp <= local_time_stamp)
                {
                    // update data
                    ++data.signal_count;
                    data.last_signal_time_stamp += timer.signal_scape;

                    // signal
                    need_remove_timer = timer.callback({ data.last_signal_time_stamp, data.signal_count });

                    if (need_remove_timer)
                    {
                        break;
                    }
                }
            }
        }

        // remove
        if (data.signal_count >= timer.repeat_count || need_remove_timer)
        {
            iter.erase_and_move_next();
        }
        else
        {
            iter.move_next();
        }
    }
}

// add/remove
TimerHandle TimerManager::add(Timer timer)
{
    return _timers.add({ std::move(timer),
                         _cur_time_stamp,
                         -1,
                         0 })
    .index();
}
void TimerManager::remove(TimerHandle handle)
{
    return _timers.remove_at(handle);
}
} // namespace skr::gui
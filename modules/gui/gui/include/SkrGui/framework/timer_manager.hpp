#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrRT/containers/sparse_vector.hpp"
#ifndef __meta__
    #include "SkrGui/framework/timer_manager.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct("guid": "f172b80f-a4b5-461c-9ac8-501e4dc732a4")
TimerSignalData {
    float    time_stamp;
    uint64_t repeat_count;
};
using TimerSignalCallback = Function<bool(TimerSignalData data)>;

sreflect_struct("guid": "063a265c-67a0-4ec1-bb26-c6a0c890c105")
Timer {

    inline static Timer OneShot(float time, TimerSignalCallback callback)
    {
        return {
            std::move(callback),
            0,
            time,
            1
        };
    }
    inline static Timer Tick(TimerSignalCallback callback)
    {
        return {
            std::move(callback),
            0,
            0,
            0
        };
    }
    inline static Timer NextTick(TimerSignalCallback callback)
    {
        return {
            std::move(callback),
            0,
            0,
            1
        };
    }
    inline static Timer Repeat(float signal_scape, float first_signal_scape, uint64_t repeat_count, TimerSignalCallback callback)
    {
        return {
            std::move(callback),
            signal_scape,
            first_signal_scape,
            repeat_count
        };
    }

    // 通知回调，输入 Timer 信息，返回是否中止 timer
    TimerSignalCallback callback = 0;

    // 每次调用间隔，0 代表下一次 tick 立即调用
    float signal_scape = 0;

    // 首次调用间隔，0 代表下一次 tick 立即调用
    float first_signal_scape = 0;

    // 重复调用次数，0 代表无限重复
    uint64_t repeat_count = 0;
};

using TimerHandle = uint64_t;

sreflect_struct("guid": "59ef6a71-e392-40b0-98d2-c7adf1b1a2e5")
SKR_GUI_API TimerManager {
    // update
    void update(float time_stamp);

    // add/remove
    TimerHandle add(Timer timer);
    void        remove(TimerHandle handle);

private:
    struct TimerData {
        Timer    timer;
        float    initial_time_stamp;
        float    last_signal_time_stamp;
        uint64_t signal_count;
    };
    float                  _cur_time_stamp = 0;
    SparseVector<TimerData> _timers;
};
} // namespace gui sreflect
} // namespace skr sreflect
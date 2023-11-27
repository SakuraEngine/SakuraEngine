#pragma once
#include "SkrGui/system/input/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture_arena.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{

sreflect_struct("guid": "952528b4-510f-450f-81fa-cbdb3d8b2d4a")
SKR_GUI_API GestureArena {
    struct ArenaData {
        Array<GestureRecognizer*> members = {};

        bool is_open           = true;  // 开启期间允许手势加入，开启期间通常指 down 事件 dispatch 期间
        bool is_held           = false; // 是否需要维持生命周期以进行例如双击等长周期的手势竞争
        bool has_pending_sweep = false; // 用于标识结束生命周期维持后，是否需要进行 sweep

        GestureRecognizer* eager_winner = nullptr;

        inline void add(GestureRecognizer* gesture)
        {
            SKR_ASSERT(is_open && "add gesture when arena is closed is not allowed");
            members.add(gesture);
        }
    };
    struct Entry {
        GestureArena*      arena   = nullptr;
        CombinePointerId   pointer = {};
        GestureRecognizer* gesture = nullptr;

        // TODO. eager_winners

        // TODO. accept/reject
    };

    inline Entry add(CombinePointerId pointer, GestureRecognizer* gesture)
    {
        auto data = _arenas.add_default(pointer);
        data->value.add(gesture);
        return { this, pointer, gesture };
    }
    inline void close(CombinePointerId pointer)
    {
        if (auto found_data = _arenas.find(pointer))
        {
            found_data->value.is_open = false;
            _try_to_resolve_arena(pointer, found_data->value);
        }
    }
    void sweep(CombinePointerId pointer)
    {
        if (auto found_data = _arenas.find(pointer))
        {
            if (found_data->value.is_held)
            {
                found_data->value.has_pending_sweep = true;
            }
            else
            {
                // 没有期望赢家，直接接受第一个手势，拒绝其余手势
                if (!found_data->value.members.empty())
                {
                    found_data->value.members[0]->accept_gesture(pointer);
                    for (uint32_t i = 0; i < found_data->value.members.size(); ++i)
                    {
                        found_data->value.members[i]->reject_gesture(pointer);
                    }
                }
                _arenas.remove(pointer);
            }
        }
    }
    void hold(CombinePointerId pointer)
    {
        if (auto found_data = _arenas.find(pointer))
        {
            found_data->value.is_held = true;
        }
    }
    void release(CombinePointerId pointer)
    {
        if (auto found_data = _arenas.find(pointer))
        {
            found_data->value.is_held = false;
            if (found_data->value.has_pending_sweep)
            {
                sweep(pointer);
            }
        }
    }

private:
    // helper functions
    inline void _resolve(CombinePointerId pointer, GestureRecognizer* member, bool accept)
    {
        if (auto found_data = _arenas.find(pointer))
        {
            if (accept)
            {
                if (found_data->value.is_open)
                {
                    found_data->value.eager_winner = member;
                }
                else
                {
                    _resolve_in_favor_of(pointer, found_data->value, member);
                }
            }
            else
            {
                found_data->value.members.remove(member);
                member->reject_gesture(pointer);
                if (!found_data->value.is_open)
                {
                    _try_to_resolve_arena(pointer, found_data->value);
                }
            }
        }
    }
    inline void _try_to_resolve_arena(CombinePointerId pointer, ArenaData& data)
    {
        if (data.members.empty()) // do noting
        {
            _arenas.remove(pointer);
        }
        else if (data.members.size() == 1) // 只有一个手势，直接接受
        {
            data.members[0]->accept_gesture(pointer);
            _arenas.remove(pointer);
        }
        else // 多个手势，接受希望胜利的 winner，如果没有这个 winner 代表暂时无法决议
        {
            if (data.eager_winner)
            {
                _resolve_in_favor_of(pointer, data, data.eager_winner);
            }
        }
    }
    inline void _resolve_in_favor_of(CombinePointerId pointer, ArenaData& data, NotNull<GestureRecognizer*> winner)
    {
        for (auto& gesture : data.members)
        {
            if (gesture != winner)
            {
                gesture->reject_gesture(pointer);
            }
        }
        winner->accept_gesture(pointer);
        _arenas.remove(pointer);
    }

private:
    UMap<CombinePointerId, ArenaData> _arenas = {};
};

} // namespace gui sreflect
} // namespace skr sreflect
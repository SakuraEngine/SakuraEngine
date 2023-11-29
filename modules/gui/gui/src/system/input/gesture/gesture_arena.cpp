#include "SkrGui/system/input/gesture/gesture_arena.hpp"

namespace skr::gui
{
GestureArena::GestureArena(CombinePointerId pointer)
    : _pointer(pointer)
{
}

void GestureArena::open()
{
    if (_state == GestureArenaState::Closed)
    {
        SKR_LOG_ERROR(u8"open a closed arena is not allowed");
    }
    _state = GestureArenaState::Opened;
}
void GestureArena::add(GestureRecognizer* gesture)
{
    SKR_ASSERT(_state == GestureArenaState::Opened && "add gesture when arena is closed is not allowed");
    _members.add(gesture);
}
void GestureArena::close()
{
    if (_state != GestureArenaState::Opened)
    {
        SKR_LOG_ERROR(u8"close a non-opened arena is not allowed");
    }
    _state = GestureArenaState::Closed;
    _try_to_resolve_arena();
}
void GestureArena::sweep(CombinePointerId pointer)
{
    if (_state != GestureArenaState::Closed)
    {
        SKR_LOG_ERROR(u8"sweep a non-closed arena is not allowed");
    }

    if (_is_held) // 延迟 Sweep 状态到 release
    {
        _has_pending_sweep = true;
    }
    else
    {
        // 没有期望赢家，直接接受第一个手势，拒绝其余手势
        if (!_members.empty())
        {
            _members[0]->accept_gesture(pointer);
            for (uint32_t i = 0; i < _members.size(); ++i)
            {
                _members[i]->reject_gesture(pointer);
            }
        }

        _state = GestureArenaState::Resolved;
    }
}
void GestureArena::hold()
{
    _is_held = true;
}
void GestureArena::release(CombinePointerId pointer)
{
    _is_held = false;
    if (_has_pending_sweep)
    {
        sweep(pointer);
    }
}
void GestureArena::accept_gesture(GestureRecognizer* member)
{
    if (_state == GestureArenaState::Opened)
    {
        eager_winner = member;
    }
    else if (_state == GestureArenaState::Closed)
    {
        _resolve_in_favor_of(member);
    }
    else
    {
        SKR_LOG_ERROR(u8"accept gesture in a resolved arena is not allowed");
    }
}
void GestureArena::reject_gesture(GestureRecognizer* member)
{
    if (_state == GestureArenaState::Resolved)
    {
        SKR_LOG_ERROR(u8"reject gesture in a resolved arena is not allowed");
    }

    _members.remove(member);
    member->reject_gesture(_pointer);
    if (_state == GestureArenaState::Closed)
    {
        _try_to_resolve_arena();
    }
}

// helper functions
void GestureArena::_try_to_resolve_arena()
{
    if (_state != GestureArenaState::Closed)
    {
        SKR_LOG_ERROR(u8"try to resolve a non-closed arena is not allowed");
    }

    if (_members.empty()) // empty arena, just remove
    {
        _state = GestureArenaState::Resolved;
    }
    else if (_members.size() == 1) // 只有一个手势，直接接受
    {
        _members[0]->accept_gesture(_pointer);
        _state = GestureArenaState::Resolved;
    }
    else if (eager_winner) // 多个手势，接受希望胜利的 winner，如果没有这个 winner 代表暂时无法决议
    {
        _resolve_in_favor_of(eager_winner);
        _state = GestureArenaState::Resolved;
    }
}
void GestureArena::_resolve_in_favor_of(GestureRecognizer* member)
{
    for (auto gesture : _members)
    {
        if (gesture != member)
        {
            gesture->reject_gesture(_pointer);
        }
    }
    member->accept_gesture(_pointer);
}

} // namespace skr::gui
#include "SkrGui/system/input/gesture/gesture_arena.hpp"

namespace skr::gui
{
void GestureArena::add(GestureRecognizer* gesture)
{
    SKR_ASSERT(_is_open && "add gesture when arena is closed is not allowed");
    _members.add(gesture);
}
bool GestureArena::close()
{
    _is_open = false;
    return _try_to_resolve_arena();
}
bool GestureArena::sweep(CombinePointerId pointer)
{
    if (_is_held) // held, mark need sweep first
    {
        _has_pending_sweep = true;
        return false;
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
        return true;
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

// helper functions
void GestureArena::_resolve(GestureRecognizer* member, bool accept)
{
    if (accept)
    {
        if (_is_open)
        {
            eager_winner = member;
        }
        else
        {
            _resolve_in_favor_of(member);
        }
    }
    else // reject
    {
        _members.remove(member);
        member->reject_gesture(_pointer);
        if (!_is_open)
        {
            _try_to_resolve_arena();
        }
    }
}
bool GestureArena::_try_to_resolve_arena()
{
    if (_members.empty()) // empty arena, just remove
    {
        return true;
    }
    else if (_members.size() == 1) // 只有一个手势，直接接受
    {
        _members[0]->accept_gesture(_pointer);
        return true;
    }
    else if (eager_winner) // 多个手势，接受希望胜利的 winner，如果没有这个 winner 代表暂时无法决议
    {
        _resolve_in_favor_of(eager_winner);
        return true;
    }
    return false;
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
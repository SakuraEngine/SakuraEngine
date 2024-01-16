#include "SkrGui/system/input/gesture/gesture_arena_manager.hpp"

namespace skr::gui
{
GestureArenaManager::~GestureArenaManager()
{
    for (auto& pair : _arenas)
    {
        SkrDelete(pair.value);
    }
}

GestureArena* GestureArenaManager::open_if_resolved(CombinePointerId pointer)
{
    GestureArena* arena = find_arena_or_add(pointer);
    if (arena->state() == GestureArenaState::Resolved)
    {
        arena->open();
    }
    return arena;
}
GestureArena* GestureArenaManager::add_gesture(CombinePointerId pointer, GestureRecognizer* gesture)
{
    GestureArena* arena = find_arena_or_add(pointer);
    arena->add(gesture);
    return arena;
}

GestureArena* GestureArenaManager::find_arena(CombinePointerId pointer)
{
    auto result = _arenas.find(pointer);
    return result ? _arenas.find(pointer).value() : nullptr;
}
GestureArena* GestureArenaManager::find_arena_or_add(CombinePointerId pointer)
{
    auto result = _arenas.try_add_default(pointer);
    if (!result.already_exist())
    {
        result.value() = SkrNew<GestureArena>(pointer);
    }
    return result.value();
}

bool GestureArenaManager::route_event(Event* event)
{
    if (auto pointer_event = event->type_cast<PointerEvent>())
    {
        auto arena    = find_arena({ pointer_event->pointer_id, static_cast<uint32_t>(pointer_event->button) });
        bool listened = false;
        if (arena)
        {
            for (auto& member : arena->members())
            {
                listened |= member->handle_event(event);
            }
        }
        return listened;
    }

    return false;
}
} // namespace skr::gui
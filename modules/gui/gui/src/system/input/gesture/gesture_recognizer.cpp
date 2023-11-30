#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"

namespace skr::gui
{
GestureRecognizer::GestureRecognizer(NotNull<InputManager*> manager)
    : _input_manager(manager)
{
}

} // namespace skr::gui
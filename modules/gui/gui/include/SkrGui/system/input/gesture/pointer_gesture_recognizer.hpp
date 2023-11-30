#pragma once
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/pointer_gesture_recognizer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct("guid": "e4db9750-96ba-4558-8a14-402d1f9a2c7a")
SKR_GUI_API PointerGestureRecognizer : public GestureRecognizer {
    SKR_RTTR_GENERATE_BODY()
};

} // namespace gui sreflect
} // namespace skr sreflect

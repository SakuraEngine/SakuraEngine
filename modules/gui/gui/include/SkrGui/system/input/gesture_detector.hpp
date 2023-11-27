#pragma once
#include "SkrGui/framework/widget/stateful_widget.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture_detector.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct GestureRecognizer;

sreflect_struct("guid": "cd94cf66-7511-418e-aef1-1e7f2dd219e7")
SKR_GUI_API RawGestureDetector : public StatefulWidget {
    SKR_RTTR_GENERATE_BODY()
    using Super                    = StatefulWidget;
    using GestureRecognizerFactory = Function<GestureRecognizer*()>;

    Widget*                              child    = nullptr;
    UMap<GUID, GestureRecognizerFactory> gestures = {};
};
} // namespace gui sreflect
} // namespace skr sreflect
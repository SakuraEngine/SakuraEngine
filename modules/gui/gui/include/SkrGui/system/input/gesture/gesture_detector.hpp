#pragma once
#include "SkrGui/framework/widget/stateful_widget.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/gesture_detector.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct GestureRecognizer;

// TODO. 隔了一层 Factory 更新逻辑不是很好写，直接穿透比较好，有特殊需求自己维护 Detector，搞泛用型干嘛呢
sreflect_struct("guid": "cd94cf66-7511-418e-aef1-1e7f2dd219e7")
SKR_GUI_API RawGestureDetector : public StatefulWidget {
    SKR_RTTR_GENERATE_BODY()
    using Super                    = StatefulWidget;
    using GestureRecognizerFactory = Function<GestureRecognizer*()>;

    Widget*                              child    = nullptr;
    Map<GUID, GestureRecognizerFactory> gestures = {};
};
} // namespace gui sreflect
} // namespace skr sreflect
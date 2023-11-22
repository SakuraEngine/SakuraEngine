#pragma once
#include "SkrGui/framework/layer/window_layer.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/native_window_layer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct INativeWindow;
sreflect_struct(
    "guid": "73726a05-07c1-4626-b608-bc78364508c5"
)
SKR_GUI_API NativeWindowLayer : public WindowLayer {
    SKR_RTTR_GENERATE_BODY()
    NativeWindowLayer(INativeWindow* native_window);
};
} // namespace gui sreflect
} // namespace skr sreflect
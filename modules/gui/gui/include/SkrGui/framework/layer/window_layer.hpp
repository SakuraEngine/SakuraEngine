#pragma once
#include "SkrGui/framework/layer/offset_layer.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/window_layer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct IWindow;
sreflect_struct(
    "guid": "4d352295-d5e8-4847-b621-9c098ed37289",
    "rtti": true
)
SKR_GUI_API WindowLayer : public OffsetLayer
{
    SKR_RTTR_GENERATE_BODY()
    WindowLayer(IWindow * window);

    void update_window();

private:
    IWindow* _window = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect
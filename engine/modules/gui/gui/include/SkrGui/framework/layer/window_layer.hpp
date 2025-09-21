#pragma once
#include "SkrGui/framework/layer/offset_layer.hpp"
#include "SkrGui/framework/layer/window_layer.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "4d352295-d5e8-4847-b621-9c098ed37289"
)]] SKR_GUI_API WindowLayer : public OffsetLayer
{
    SKR_GENERATE_BODY(WindowLayer)
    WindowLayer(INativeWindow* window);

    void update_window();

private:
    INativeWindow* _window = nullptr;
};
} // namespace skr::gui
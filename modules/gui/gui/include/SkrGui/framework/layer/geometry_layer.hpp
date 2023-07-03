#pragma once
#include "SkrGui/framework/layer/layer.hpp"

namespace skr::gui
{
struct GeometryLayer : public Layer {
    SKR_GUI_OBJECT(GeometryLayer, "8594695b-da35-462a-b4bf-b4ab891088c5", Layer)

    inline ICanvas* canvas() const SKR_NOEXCEPT { return _canvas; }

private:
    ICanvas* _canvas = nullptr;
};
} // namespace skr::gui
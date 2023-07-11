#pragma once
#include "SkrGui/framework/layer/container_layer.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct SKR_GUI_API OffsetLayer : public ContainerLayer {
    SKR_GUI_OBJECT(OffsetLayer, "739c87d9-0e84-4300-a15e-cf892ca8379f", ContainerLayer)

    inline void    set_offset(Offsetf offset) noexcept { _offset = offset; }
    inline Offsetf offset() const noexcept { return _offset; }

private:
    Offsetf _offset = {};
};
} // namespace skr::gui
#pragma once
#include "SkrGui/framework/layer/container_layer.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/layer/offset_layer.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "67c40c35-4cb7-48f1-b790-e29fe843c29a"
)]] SKR_GUI_API OffsetLayer : public ContainerLayer
{
    SKR_GENERATE_BODY(OffsetLayer)

    inline void set_offset(Offsetf offset) noexcept { _offset = offset; }
    inline Offsetf offset() const noexcept { return _offset; }

private:
    Offsetf _offset = {};
};
} // namespace skr::gui
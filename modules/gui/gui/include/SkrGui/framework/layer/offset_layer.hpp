#pragma once
#include "SkrGui/framework/layer/container_layer.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/offset_layer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "67c40c35-4cb7-48f1-b790-e29fe843c29a",
    "rtti": true
)
SKR_GUI_API OffsetLayer : public ContainerLayer
{
    SKR_RTTR_GENERATE_BODY()

    inline void    set_offset(Offsetf offset) noexcept { _offset = offset; }
    inline Offsetf offset() const noexcept { return _offset; }

private:
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Offsetf _offset = {};
};
} // namespace gui sreflect
} // namespace skr sreflect
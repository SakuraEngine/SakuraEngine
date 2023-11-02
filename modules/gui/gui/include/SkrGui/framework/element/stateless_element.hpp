#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/stateless_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "05699161-383d-481e-abfa-ce0a7110dc2c",
    "rtti": true
)
SKR_GUI_API StatelessElement : public ComponentElement{
    SKR_RTTR_GENERATE_BODY()
};
} // namespace gui sreflect
} // namespace skr sreflect
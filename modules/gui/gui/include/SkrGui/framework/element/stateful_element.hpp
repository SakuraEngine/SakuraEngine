#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/stateful_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "de275a65-9b5a-4267-bc6e-355c32ca6a22",
    "rtti": true
)
SKR_GUI_API StatefulElement : public ComponentElement{
    SKR_RTTR_GENERATE_BODY()
};
} // namespace gui sreflect
} // namespace skr sreflect
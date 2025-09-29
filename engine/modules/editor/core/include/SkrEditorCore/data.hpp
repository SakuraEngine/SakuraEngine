#pragma once
#include <SkrBase/config.h>
#include <SkrRTTR/irttr_basic.hpp>
#include <SkrEditorCore/data.generated.h>

namespace skr
{
struct [[sattr(guid = "3598a784-9ee0-4d02-8fe9-839cda1effc6"
)]] SKR_EDITOR_CORE_API EdData : virtual IRTTRBasic
{
    SKR_GENERATE_BODY(EdData)
};
} // namespace skr
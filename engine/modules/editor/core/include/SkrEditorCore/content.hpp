#pragma once
#include <SkrBase/config.h>
#include <SkrRTTR/irttr_basic.hpp>
#include <SkrEditorCore/content.generated.h>

namespace skr
{
struct [[sattr(guid = "1aace33a-f09c-4a6b-928b-33e325ccc12f"
)]] SKR_EDITOR_CORE_API EdContent : virtual IRTTRBasic
{
    SKR_GENERATE_BODY(EdContent)
};
} // namespace skr
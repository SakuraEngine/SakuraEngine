#pragma once
#include <SkrBase/config.h>
#include <SkrRTTR/iobject.hpp>
#ifndef __meta__
    #include <SkrEditorCore/content.generated.h>
#endif

namespace skr
{
// clang-format off
sreflect_struct(guid = "1aace33a-f09c-4a6b-928b-33e325ccc12f")
SKR_EDITOR_CORE_API EdContent : virtual IObject {
    // clang-format on
    SKR_GENERATE_BODY()
};
} // namespace skr
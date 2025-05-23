#pragma once
#include <SkrBase/config.h>
#include <SkrRTTR/iobject.hpp>
#ifndef __meta__
    #include <SkrEditorCore/data.generated.h>
#endif

namespace skr
{
// clang-format off
sreflect_struct(guid = "3598a784-9ee0-4d02-8fe9-839cda1effc6")
SKR_EDITOR_CORE_API EdData : virtual IObject {
    // clang-format on
    SKR_GENERATE_BODY()
};
} // namespace skr
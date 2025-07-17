#pragma once
#include <SkrContainers/string.hpp>
#include <SkrRTTR/iobject.hpp>
#include <SkrCore/memory/rc.hpp>

#ifndef __meta__
    #include "SkrV8/v8_script_loader.generated.h"
#endif

namespace skr
{
// clang-format off
sreflect_struct(guid = "71d1b125-2e17-4a92-b162-64297f0d41e5")
SKR_V8_API IV8ScriptLoader : virtual IObject {
    // clang-format on
    SKR_GENERATE_BODY()
    SKR_RC_IMPL();

    virtual String           normalize_path(String raw_path) = 0;
    virtual Optional<String> load_script(String path)        = 0;
};

// clang-format off
sreflect_struct(guid = "e2db63ac-86d4-474a-8c79-46566a1f7135")
SKR_V8_API V8ScriptLoaderSystemFS : IV8ScriptLoader {
    // clang-format on
    SKR_GENERATE_BODY()

    String           normalize_path(String raw_path) override;
    Optional<String> load_script(String path) override;
};

} // namespace skr
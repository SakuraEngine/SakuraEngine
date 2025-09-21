#pragma once
#include <SkrContainers/string.hpp>
#include <SkrRTTR/iobject.hpp>
#include <SkrCore/memory/rc.hpp>

#include "SkrV8/v8_vfs.generated.h"

namespace skr
{
struct [[sattr(
    guid = "71d1b125-2e17-4a92-b162-64297f0d41e5"
)]] SKR_V8_API IV8VFS : virtual IObject
{
    SKR_GENERATE_BODY(IV8VFS)
    SKR_RC_IMPL();

    // path operations
    virtual String path_normalize(String raw_path) = 0;
    virtual String path_solve_relative_module(String referrer, String relative_path) = 0;

    // io operations
    virtual Optional<String> load_script(String path) = 0;
};

struct [[sattr(
    guid = "e2db63ac-86d4-474a-8c79-46566a1f7135"
)]] SKR_V8_API V8VFSSystemFS : IV8VFS
{
    // clang-format on
    SKR_GENERATE_BODY(V8VFSSystemFS)

    // path operations
    String path_normalize(String raw_path) override;
    String path_solve_relative_module(String referrer, String relative_path) override;

    // io operations
    Optional<String> load_script(String path) override;

    String root_path = {};
};

} // namespace skr
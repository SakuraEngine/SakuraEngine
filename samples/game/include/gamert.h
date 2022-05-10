#pragma once
#include "gamert_configure.h"
#include "platform/vfs.h"

namespace skg
{
struct GAMERT_API Core {
    static skr_vfs_t* resource_vfs;
    static void initialize();
};

} // namespace skg
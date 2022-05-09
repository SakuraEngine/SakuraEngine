#pragma once
#include "gamert_configure.h"
#include "platform/vfs.h"

namespace skg
{
struct GAMERT_API Core {
    GAMERT_API static skr_vfs_t* resource_vfs;
    GAMERT_API static void initialize();
};

} // namespace skg
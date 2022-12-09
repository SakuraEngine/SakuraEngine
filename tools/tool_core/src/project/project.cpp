#include "SkrToolCore/project/project.hpp"
#include "platform/vfs.h"
#include "utils/io.h"

namespace skd
{
SProject::~SProject() noexcept
{
    if(ram_service) skr_io_ram_service_t::destroy(ram_service);
    if (vfs) skr_free_vfs(vfs);
}
}
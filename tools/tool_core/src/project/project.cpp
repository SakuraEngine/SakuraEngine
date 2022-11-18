#include "SkrToolCore/project/project.hpp"
#include "platform/vfs.h"

namespace skd
{
SProject::~SProject() noexcept
{
    if (vfs) skr_free_vfs(vfs);
}
}
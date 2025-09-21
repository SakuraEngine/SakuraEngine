#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr
{
void Serialize<skr::SkeletonResource>::read(skr::ArchiveRead& r, skr::SkeletonResource& v)
{
    ozz::io::SkrStream stream(&r, nullptr);
    ozz::io::IArchive archive(&stream);
    archive >> v.skeleton;
}
void Serialize<skr::SkeletonResource>::write(skr::ArchiveWrite& w, const skr::SkeletonResource& v)
{
    ozz::io::SkrStream stream(nullptr, &w);
    ozz::io::OArchive archive(&stream);
}
} // namespace skr

namespace skr
{
GUID SkelFactory::GetResourceType()
{
    return ::skr::type_id_of<skr::SkeletonResource>();
}
} // namespace skr
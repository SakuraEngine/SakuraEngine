#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr
{
bool BinSerde<skr::anim::SkeletonResource>::read(SBinaryReader* r, skr::anim::SkeletonResource& v)
{
    ozz::io::SkrStream stream(r, nullptr);
    ozz::io::IArchive  archive(&stream);
    archive >> v.skeleton;
    return true;
}
bool BinSerde<skr::anim::SkeletonResource>::write(SBinaryWriter* w, const skr::anim::SkeletonResource& v)
{
    ozz::io::SkrStream stream(nullptr, w);
    ozz::io::OArchive  archive(&stream);
    archive << v.skeleton;
    return true;
}
} // namespace skr

namespace skr::resource
{
skr_guid_t SSkelFactory::GetResourceType()
{
    return ::skr::rttr::type_id_of<skr::anim::SkeletonResource>();
}
} // namespace skr::resource
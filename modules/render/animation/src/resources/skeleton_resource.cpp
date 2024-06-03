#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr::binary
{
bool ReadTrait<skr::anim::SkeletonResource>::Read(SBinaryReader* reader, skr::anim::SkeletonResource& value)
{
    ozz::io::SkrStream stream(reader, nullptr);
    ozz::io::IArchive  archive(&stream);
    archive >> value.skeleton;
    return true;
}
bool WriteTrait<skr::anim::SkeletonResource>::Write(SBinaryWriter* writer, const skr::anim::SkeletonResource& value)
{
    ozz::io::SkrStream stream(nullptr, writer);
    ozz::io::OArchive  archive(&stream);
    archive << value.skeleton;
    return true;
}
} // namespace skr::binary

namespace skr::resource
{
skr_guid_t SSkelFactory::GetResourceType()
{
    return ::skr::rttr::type_id_of<skr::anim::SkeletonResource>();
}
} // namespace skr::resource
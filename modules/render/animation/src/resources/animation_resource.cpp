#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr::binary
{
bool ReadTrait<anim::AnimResource>::Read(SBinaryReader* reader, anim::AnimResource& value)
{
    ozz::io::SkrStream stream(reader, nullptr);
    ozz::io::IArchive  archive(&stream);
    archive >> value.animation;
    return true;
}
bool WriteTrait<anim::AnimResource>::Write(SBinaryWriter* writer, const anim::AnimResource& value)
{
    ozz::io::SkrStream stream(nullptr, writer);
    ozz::io::OArchive  archive(&stream);
    archive << value.animation;
    return true;
}
} // namespace skr::binary

namespace skr::resource
{
skr_guid_t SAnimFactory::GetResourceType()
{
    return skr::rttr::type_id_of<anim::AnimResource>();
}
} // namespace skr::resource
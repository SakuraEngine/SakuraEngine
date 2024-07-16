#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr
{
bool BinSerde<anim::AnimResource>::read(SBinaryReader* r, anim::AnimResource& v)
{
    ozz::io::SkrStream stream(r, nullptr);
    ozz::io::IArchive  archive(&stream);
    archive >> v.animation;
    return true;
}
bool BinSerde<anim::AnimResource>::write(SBinaryWriter* w, const anim::AnimResource& v)
{
    ozz::io::SkrStream stream(nullptr, w);
    ozz::io::OArchive  archive(&stream);
    archive << v.animation;
    return true;
}
} // namespace skr

namespace skr::resource
{
skr_guid_t SAnimFactory::GetResourceType()
{
    return skr::rttr::type_id_of<anim::AnimResource>();
}
} // namespace skr::resource
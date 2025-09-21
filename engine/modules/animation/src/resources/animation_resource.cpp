#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr
{
void Serialize<AnimResource>::read(skr::ArchiveRead& r, AnimResource& v)
{
    ozz::io::SkrStream stream(&r, nullptr);
    ozz::io::IArchive archive(&stream);
    archive >> v.animation;
}
void Serialize<AnimResource>::write(skr::ArchiveWrite& w, const AnimResource& v)
{
    ozz::io::SkrStream stream(nullptr, &w);
    ozz::io::OArchive archive(&stream);
    archive << v.animation;
}
} // namespace skr

namespace skr
{
GUID AnimFactory::GetResourceType()
{
    return skr::type_id_of<AnimResource>();
}
} // namespace skr
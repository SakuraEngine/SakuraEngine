#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr::binary
{
int ReadTrait<anim::AnimResource>::Read(skr_binary_reader_t* reader, anim::AnimResource& value)
{
    ozz::io::SkrStream stream(reader, nullptr);
    ozz::io::IArchive  archive(&stream);
    archive >> value.animation;
    return 0;
}
int WriteTrait<anim::AnimResource>::Write(skr_binary_writer_t* writer, const anim::AnimResource& value)
{
    ozz::io::SkrStream stream(nullptr, writer);
    ozz::io::OArchive  archive(&stream);
    archive << value.animation;
    return 0;
}
} // namespace skr::binary

namespace skr::resource
{
skr_guid_t SAnimFactory::GetResourceType()
{
    return skr::rttr::type_id<anim::AnimResource>();
}
} // namespace skr::resource
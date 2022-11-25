#include "SkrAnim/resources/animation_resource.h"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr::binary
{
    int ReadHelper<skr_anim_resource_t>::Read(skr_binary_reader_t *reader, skr_anim_resource_t &value)
    {
        ozz::io::SkrStream stream(reader, nullptr);
        ozz::io::IArchive archive(&stream);
        archive >> value.animation;
        return 0;
    }
}

namespace skr::resource
{
    skr_type_id_t SAnimFactory::GetResourceType()
    {
        return type::type_id<skr_anim_resource_t>::get();
    }
}
#include "SkrScene/resources/scene_resource.h"

namespace skr::resource
{
skr_guid_t SSceneFactory::GetResourceType()
{
    return { 0xEFBA637E, 0xE7E5, 0x4B64, { 0xBA, 0x26, 0x90, 0xAE, 0xEE, 0x9E, 0x3E, 0x1A } };
}
} // namespace skr::resource

namespace skr::binary
{
int ReadTrait<skr_scene_resource_t>::Read(skr_binary_reader_t* reader, skr_scene_resource_t& value)
{
    // TODO: error code?
    value.storage = sugoiS_create();
    sugoiS_deserialize(value.storage, reader);
    return 0;
}
int WriteTrait<skr_scene_resource_t>::Write(skr_binary_writer_t* writer, const skr_scene_resource_t& value)
{
    sugoiS_serialize(value.storage, writer);
    return 0;
}
} // namespace skr::binary
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
bool ReadTrait<skr_scene_resource_t>::Read(SBinaryReader* reader, skr_scene_resource_t& value)
{
    // TODO: error code?
    value.storage = sugoiS_create();
    sugoiS_deserialize(value.storage, reader);
    return true;
}

bool WriteTrait<skr_scene_resource_t>::Write(SBinaryWriter* writer, const skr_scene_resource_t& value)
{
    sugoiS_serialize(value.storage, writer);
    return true;
}
} // namespace skr::binary
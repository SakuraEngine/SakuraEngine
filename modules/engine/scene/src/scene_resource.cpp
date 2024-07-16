#include "SkrScene/resources/scene_resource.h"

namespace skr::resource
{
skr_guid_t SSceneFactory::GetResourceType()
{
    return { 0xEFBA637E, 0xE7E5, 0x4B64, { 0xBA, 0x26, 0x90, 0xAE, 0xEE, 0x9E, 0x3E, 0x1A } };
}
} // namespace skr::resource

namespace skr
{
bool BinSerde<skr_scene_resource_t>::read(SBinaryReader* r, skr_scene_resource_t& v)
{
    // TODO: error code?
    v.storage = sugoiS_create();
    sugoiS_deserialize(v.storage, r);
    return true;
}

bool BinSerde<skr_scene_resource_t>::write(SBinaryWriter* w, const skr_scene_resource_t& v)
{
    sugoiS_serialize(v.storage, w);
    return true;
}
} // namespace skr
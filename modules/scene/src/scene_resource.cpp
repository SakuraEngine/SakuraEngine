#include "SkrScene/resources/scene_resource.h"

namespace skr::resource
{
    skr_type_id_t SSceneFactory::GetResourceType()
    {
        return {0x2DEFA834, 0x5CF3, 0x45A5, {0x86, 0x7E, 0x8E, 0x0E, 0xD0, 0xF7, 0xD6, 0xAE}};
    }
}

namespace skr::binary
{
int ReadTrait<skr_scene_resource_t>::Read(skr_binary_reader_t* reader, skr_scene_resource_t& value)
{
    //TODO: error code?
    dualS_deserialize(value.storage, reader);
    return 0;
}
int WriteTrait<const skr_scene_resource_t&>::Write(skr_binary_writer_t* writer, const skr_scene_resource_t& value)
{
    dualS_serialize(value.storage, writer);
    return 0;
}
} // namespace skr::binary
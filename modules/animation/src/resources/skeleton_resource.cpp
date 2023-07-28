#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/ozz/base/io/archive.h"

namespace skr::binary
{
    int ReadTrait<skr_skeleton_resource_t>::Read(skr_binary_reader_t *reader, skr_skeleton_resource_t &value)
    {
        ozz::io::SkrStream stream(reader, nullptr);
        ozz::io::IArchive archive(&stream);
        archive >> value.skeleton;
        return 0;
    }
    int WriteTrait<const skr_skeleton_resource_t &>::Write(skr_binary_writer_t* writer, const skr_skeleton_resource_t &value)
    {
        ozz::io::SkrStream stream(nullptr, writer);
        ozz::io::OArchive archive(&stream);
        archive << value.skeleton;
        return 0;
    }
}

namespace skr::resource
{
    skr_type_id_t SSkelFactory::GetResourceType()
    {
        return type::type_id<skr_skeleton_resource_t>::get();
    }
}
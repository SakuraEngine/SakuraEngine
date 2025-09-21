#include "project.h"
#include "SkrCore/serialize/json_archive.hpp"
#include "SkrCore/serialize/binary_archive.hpp"
#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrContainers/span.hpp>

void SerdeProject::Initialize(skr::String _name, skr::String _path) SKR_NOEXCEPT
{
    proj_name = _name;
    proj_path = _path;
    // create path if not exists
    auto proj_f_name = proj_name;
    proj_f_name.append(u8".sproject");
    proj_f_path = skr::Path(proj_path) / proj_f_name;
    scene_f_path = skr::Path(proj_path) / u8"scene.json";
    world_f_path = skr::Path(proj_path) / u8"world.bin";
}

bool SerdeProject::LoadAssetMeta(const skd::URI& uri, skr::String& content) noexcept
{
    if (MetaDatabase.contains(uri))
    {
        content = MetaDatabase[uri];
        return true;
    }
    return false;
}

bool SerdeProject::SaveAssetMeta(const skd::URI& uri, const skr::String& content) noexcept
{
    MetaDatabase[uri] = content;
    return true;
}

bool SerdeProject::ExistImportedAsset(const skd::URI& uri)
{
    return MetaDatabase.contains(uri);
}

void SerdeProject::SaveToDisk()
{
}

void SerdeProject::LoadFromDisk()
{
}

void SerdeProject::serialize()
{
    scene.serialize();

    // write scene
    {
        auto json_writer = skr::ArWriteJson::Create();
        json_writer.value(scene);
        json_writer.write_to_file(scene_f_path);
    }

    // write world
    {
        skr::ArWriteBin bin_writer;
        auto root = skr::Actor::GetRoot();
        auto* world = root.lock()->GetWorld();
        bin_writer.value(*world);
        skr::fs::File::write_all_bytes(world_f_path, bin_writer.buffer());
    }

    SKR_LOG_INFO(u8"SerdeProject Serialized!");
}

void SerdeProject::deserialize()
{
    // read scene
    {
        skr::ArReadJson json_reader = skr::ArReadJson::ReadFile(scene_f_path);
        json_reader.value(scene);
        scene.deserialize();
    }

    // read world
    {
        skr::Vector<uint8_t> buffer;
        skr::fs::File::read_all_bytes(world_f_path, buffer);
        skr::ArReadBin bin_reader;
        bin_reader.use_buffer(buffer);
        auto root = skr::Actor::GetRoot();
        auto* world = root.lock()->GetWorld();
        bin_reader.value(*world);
    }

    SKR_LOG_INFO(u8"SerdeProject Deserialized!");
}

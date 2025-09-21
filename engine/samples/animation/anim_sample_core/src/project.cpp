#include "AnimSampleCore/project.h"
#include "SkrContainers/hashmap.hpp"
#include <SkrCore/serialize/json_archive.hpp>

namespace skr
{

void AnimSampleProject::Initialize(skr::String _name, skr::String _path) SKR_NOEXCEPT
{
    proj_name = _name;
    proj_path = _path;
    // create path if not exists
    auto proj_f_name = proj_name;
    proj_f_name.append(u8".sproject");
    proj_f_path = skr::Path(proj_path) / proj_f_name;
}

bool AnimSampleProject::LoadAssetMeta(const skd::URI& uri, skr::String& content) noexcept
{
    if (MetaDatabase.contains(uri))
    {
        content = MetaDatabase[uri];
        return true;
    }
    return false;
}

bool AnimSampleProject::SaveAssetMeta(const skd::URI& uri, const skr::String& content) noexcept
{
    MetaDatabase[uri] = content;
    return true;
}

bool AnimSampleProject::ExistImportedAsset(const skd::URI& uri)
{
    return MetaDatabase.contains(uri);
}

void AnimSampleProject::SaveToDisk()
{
    auto writer = skr::ArWriteJson::Create();
    {
        skr::Archive::ObjectScope obj_scope(writer);
        SKR_FAST_CHECK(obj_scope.is_success(), );
        SKR_FAST_CHECK(writer.key_value(u8"assets", MetaDatabase), );
    }
    
    skr::String json_str;
    writer.write_to_string(json_str);

    if (skr::fs::File::write_all_text(proj_f_path, json_str.view()))
        SKR_LOG_INFO(u8"[AnimSampleProject] Project saved to: %s", proj_f_path.c_str());
    else
        SKR_LOG_ERROR(u8"[AnimSampleProject] Failed to save project to: %s", proj_f_path.c_str());
}

void AnimSampleProject::LoadFromDisk()
{
    auto reader = skr::ArReadJson::ReadFile(proj_f_path);
    if (reader.is_failed())
    {
        // File doesn't exist, which is fine - just means empty project
        MetaDatabase.clear();
        SKR_LOG_INFO(u8"[AnimSampleProject] No existing project file found at: %s, starting with empty project", proj_f_path.c_str());
    }

    skr::Archive::ObjectScope obj_scope(reader);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    SKR_FAST_CHECK(reader.key_value(u8"assets", MetaDatabase), );
    SKR_LOG_INFO(u8"[AnimSampleProject] Project loaded from: %s, %zu assets", proj_f_path.c_str(), MetaDatabase.size());
}

} // namespace skr
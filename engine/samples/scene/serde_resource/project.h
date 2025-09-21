#include "SkrOS/filesystem.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrContainersDef/string.hpp"

#include "SkrScene/scene.h"

struct SerdeProject : skd::SProject
{
    void Initialize(skr::String _name, skr::String _path) SKR_NOEXCEPT;
    bool LoadAssetMeta(const skd::URI& uri, skr::String& content) noexcept override;
    bool SaveAssetMeta(const skd::URI& uri, const skr::String& content) noexcept override;

    bool ExistImportedAsset(const skd::URI& uri);
    void SaveToDisk();
    void LoadFromDisk();

    void serialize();
    void deserialize();

    skr::String proj_name;
    skr::String proj_path;
    skr::Path proj_f_path;
    skr::Path scene_f_path;
    skr::Path world_f_path;
    skr::Scene scene;
    skr::ParallelFlatHashMap<skd::URI, skr::String, skr::Hash<skd::URI>> MetaDatabase;
};

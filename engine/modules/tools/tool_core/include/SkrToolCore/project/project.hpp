#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "SkrContainers/path.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrToolCore/project/project.generated.h" // IWYU pragma: export

namespace skr
{

using URI = skr::Path;

struct [[sattr(
    guid = "D153957A-2272-45F8-92DA-EEEB67821D20" 
    serde = @enable
)]] SProjectConfig
{
    skr::Path assetDirectory;
    skr::Path resourceDirectory;
    skr::Path artifactsDirectory;
};

struct TOOL_CORE_API SProject
{
public:
    ~SProject() noexcept;

    virtual bool LoadAssetMeta(const URI& uri, skr::String& content) noexcept;
    virtual bool SaveAssetMeta(const URI& uri, const skr::String& content) noexcept;
    virtual bool LoadAssetSourceFile(const URI& uri, skr::Vector<uint8_t>& content) noexcept;

    virtual bool OpenProject(const URI& path) noexcept;
    virtual bool OpenProject(const skr::String& name, const skr::Path& root, const SProjectConfig& config) noexcept;
    virtual bool CloseProject() noexcept;

    virtual skr::String GetEnv(skr::StringView name);
    virtual void SetEnv(skr::String, skr::String);

    skr_vfs_t* GetAssetVFS() const;
    skr_vfs_t* GetResourceVFS() const;
    skr_vfs_t* GetDependencyVFS() const;
    skr::io::IRAMService* GetRamService() const;

    URI GetAssetPath() const noexcept { return assetDirectory; }
    URI GetOutputPath() const noexcept { return resourceDirectory; }
    URI GetDependencyPath() const noexcept { return dependencyDirectory; }

private:
    skr_vfs_t* asset_vfs = nullptr;
    skr_vfs_t* resource_vfs = nullptr;
    skr_vfs_t* dependency_vfs = nullptr;
    skr::io::IRAMService* ram_service = nullptr;

    URI assetDirectory;
    URI artifactsDirectory;
    URI resourceDirectory;
    URI dependencyDirectory;
    skr::String name;
    skr::ParallelFlatHashMap<skr::String, skr::String, skr::Hash<skr::String>> env;
};
} // namespace skr
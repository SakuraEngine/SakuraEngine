#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "platform/filesystem.hpp"
#include "containers/string.hpp"
#ifndef __meta__
#include "SkrToolCore/project/project.generated.h"
#endif

namespace skd sreflect
{

sreflect_struct("guid" : "D153957A-2272-45F8-92DA-EEEB67821D20")
sattr("serialize" : "json")
SProjectConfig
{
    skr::string assetDirectory;
    skr::string resourceDirectory;
    skr::string artifactsDirectory;
};

struct TOOL_CORE_API SProject 
{
private:
    skr::filesystem::path assetPath;
    skr::filesystem::path outputPath;
    skr::filesystem::path artifactsPath;
    skr::filesystem::path dependencyPath;
    skr::string name;
public:
    skr::filesystem::path GetAssetPath() const noexcept { return assetPath; }
    skr::filesystem::path GetOutputPath() const noexcept { return outputPath; }
    skr::filesystem::path GetDependencyPath() const noexcept { return dependencyPath; }

    static SProject* OpenProject(const skr::filesystem::path& path) noexcept;
    static void SetWorkspace(const skr::filesystem::path& path) noexcept;

    skr_vfs_t* asset_vfs = nullptr;
    skr_vfs_t* resource_vfs = nullptr;
    skr_io_ram_service_t* ram_service = nullptr;
    ~SProject() noexcept;
};
}
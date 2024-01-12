#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "SkrRT/platform/filesystem.hpp"
#include "SkrContainers/string.hpp"
#ifndef __meta__
#include "SkrToolCore/project/project.generated.h" // IWYU pragma: export
#endif

namespace skd sreflect
{

sreflect_struct("guid" : "D153957A-2272-45F8-92DA-EEEB67821D20")
sattr("serialize" : "json")
SProjectConfig
{
    skr::String assetDirectory;
    skr::String resourceDirectory;
    skr::String artifactsDirectory;
};

struct TOOL_CORE_API SProject 
{
private:
    skr::filesystem::path assetPath;
    skr::filesystem::path outputPath;
    skr::filesystem::path artifactsPath;
    skr::filesystem::path dependencyPath;
    skr::String name;
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
#include "SkrToolCore/project/project.hpp"
#include "misc/log.h"
#include "platform/vfs.h"
#include "misc/io.h"
#include "simdjson.h"
#include "serde/json/reader.h"
#include "misc/make_zeroed.hpp"

namespace skd
{
static skr::filesystem::path Workspace;
void SProject::SetWorkspace(const skr::filesystem::path& path) noexcept
{
    Workspace = path;
}
SProject* SProject::OpenProject(const skr::filesystem::path& projectFile) noexcept
{
    std::error_code ec = {};
    auto resolvePath = [&](skr::string& path)
    {
        auto view = path.view();
        auto i = view.index_of(u8"${");
        if(i == skr::text::index_invalid)
            return path;
        skr::string resolved;
        resolved.raw().reserve(path.raw().size());
        //resolved.append(view);
        while(true)
        {
            resolved.append(view.subview({'[', 0, i, ')'}));
            auto j = view.index_of(u8"}", {'[', i, view.size(), ')'});
            if (j == skr::text::index_invalid)
            {
                resolved.append(view.subview({'[', i, view.size(), ')'}));
                break;
            }
            auto var = view.subview({'[', i + 2, j, ')'});
            if (var == u8"workspace")
            {
                resolved.append(Workspace.u8string().c_str());
            }
            else if(var == u8"platform")
            {
                resolved.append(SKR_RESOURCE_PLATFORM);
            }
            view = view.subview({'[', j + 1, view.size(), ')'});
            i = view.index_of(u8"${");
            if (i == skr::text::index_invalid)
            {
                resolved.append(view);
                break;
            }
        }
        return resolved;
    };
    auto toAbsolutePath = [&](skr::string& path)
    {
        auto resolved = resolvePath(path);
        skr::filesystem::path result{resolved.c_str()};
        if (result.is_relative())
        {
            result = projectFile.parent_path() / result;
        }
        return result.lexically_normal();
    };

    auto projectPath = projectFile.lexically_normal().string();
    auto jsonstring = simdjson::padded_string::load(projectPath);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonstring);
    if(doc.error())
    {
        SKR_LOG_ERROR("Failed to parse project file: %s, error: %s", projectPath.c_str(), simdjson::error_message(doc.error()));
        return nullptr;
    }
    auto json_value = doc.get_value().value_unsafe();
    skd::SProjectConfig cfg;
    if(skr::json::Read(std::move(json_value), cfg) != skr::json::SUCCESS)
    {
        SKR_LOG_ERROR("Failed to parse project file: %s", projectPath.c_str());
        return nullptr;
    }
    
    auto project = SkrNew<skd::SProject>();
    project->name = projectFile.filename().u8string().c_str();

    project->assetPath = toAbsolutePath(cfg.assetDirectory);
    project->outputPath = toAbsolutePath(cfg.resourceDirectory);
    project->artifactsPath = toAbsolutePath(cfg.artifactsDirectory);
    project->dependencyPath = project->artifactsPath / "deps";

    // create resource VFS
    skr_vfs_desc_t resource_vfs_desc = {};
    resource_vfs_desc.app_name = project->name.u8_str();
    resource_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto u8outputPath = project->outputPath.u8string();
    resource_vfs_desc.override_mount_dir = u8outputPath.c_str();
    project->resource_vfs = skr_create_vfs(&resource_vfs_desc);

    // create asset VFS
    skr_vfs_desc_t asset_vfs_desc = {};
    asset_vfs_desc.app_name = project->name.u8_str();
    asset_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto u8assetPath = project->assetPath.u8string();
    asset_vfs_desc.override_mount_dir = u8assetPath.c_str();
    project->asset_vfs = skr_create_vfs(&asset_vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = u8"CompilerRAMIOService";
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    project->ram_service = skr_io_ram_service_t::create(&ioServiceDesc);

    return project;
}

SProject::~SProject() noexcept
{
    if(ram_service) skr_io_ram_service_t::destroy(ram_service);
    if (resource_vfs) skr_free_vfs(resource_vfs);
    if (asset_vfs) skr_free_vfs(asset_vfs);
}
}
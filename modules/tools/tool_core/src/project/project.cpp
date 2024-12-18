#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrSerde/json_serde.hpp"

namespace skd
{
static skr::filesystem::path Workspace;
void                         SProject::SetWorkspace(const skr::filesystem::path& path) noexcept
{
    Workspace = path;
}

SProject* SProject::OpenProject(const skr::filesystem::path& projectFilePath) noexcept
{
    std::error_code ec          = {};
    auto            resolvePath = [&](skr::String& path) {
        auto view = path.view();
        auto i    = view.index_of(u8"${");
        if (i == ostr::global_constant::INDEX_INVALID)
            return path;
        skr::String resolved;
        resolved.raw().reserve(path.raw().size());
        // resolved.append(view);
        while (true)
        {
            resolved.append(view.subview(0, i - 1));
            auto j = view.index_of(u8"}", i, view.size() - 1);
            if (j == ostr::global_constant::INDEX_INVALID)
            {
                resolved.append(view.subview(i, view.size() - 1));
                break;
            }
            auto var = view.subview(i + 2, j - 1);
            if (var == u8"workspace")
            {
                resolved.append(Workspace.u8string().c_str());
            }
            else if (var == u8"platform")
            {
                resolved.append(SKR_RESOURCE_PLATFORM);
            }
            view = view.subview(j + 1, view.size() - 1);
            i    = view.index_of(u8"${");
            if (i == ostr::global_constant::INDEX_INVALID)
            {
                resolved.append(view);
                break;
            }
        }
        return resolved;
    };
    auto toAbsolutePath = [&](skr::String& path) {
        auto                  resolved = resolvePath(path);
        skr::filesystem::path result{ resolved.c_str() };
        if (result.is_relative())
        {
            result = projectFilePath.parent_path() / result;
        }
        return result.lexically_normal();
    };

    auto                projectPath = projectFilePath.lexically_normal().string();
    skd::SProjectConfig cfg;
    // TODO: refactor this
    {
        auto projectFile = fopen(projectPath.c_str(), "rb");
        if (!projectFile)
        {
            SKR_LOG_ERROR(u8"Failed to open project file: %s", projectPath.c_str());
            return nullptr;
        }
        // read string from file with c <file>
        fseek(projectFile, 0, SEEK_END);
        auto fileSize = ftell(projectFile);
        fseek(projectFile, 0, SEEK_SET);
        skr::String projectFileContent;
        projectFileContent.append(u8'0', fileSize);
        fread(projectFileContent.raw().data(), 1, fileSize, projectFile);
        fclose(projectFile);

        skr::archive::JsonReader reader(projectFileContent.view());
        if (!skr::json_read(&reader, cfg))
        {
            SKR_LOG_ERROR(u8"Failed to parse project file: %s", projectPath.c_str());
            return nullptr;
        }
    }

    auto project  = SkrNew<skd::SProject>();
    project->name = projectFilePath.filename().u8string().c_str();

    project->assetPath      = toAbsolutePath(cfg.assetDirectory);
    project->outputPath     = toAbsolutePath(cfg.resourceDirectory);
    project->artifactsPath  = toAbsolutePath(cfg.artifactsDirectory);
    project->dependencyPath = project->artifactsPath / "deps";

    // create resource VFS
    skr_vfs_desc_t resource_vfs_desc     = {};
    resource_vfs_desc.app_name           = project->name.u8_str();
    resource_vfs_desc.mount_type         = SKR_MOUNT_TYPE_CONTENT;
    auto u8outputPath                    = project->outputPath.u8string();
    resource_vfs_desc.override_mount_dir = u8outputPath.c_str();
    project->resource_vfs                = skr_create_vfs(&resource_vfs_desc);

    // create asset VFS
    skr_vfs_desc_t asset_vfs_desc     = {};
    asset_vfs_desc.app_name           = project->name.u8_str();
    asset_vfs_desc.mount_type         = SKR_MOUNT_TYPE_CONTENT;
    auto u8assetPath                  = project->assetPath.u8string();
    asset_vfs_desc.override_mount_dir = u8assetPath.c_str();
    project->asset_vfs                = skr_create_vfs(&asset_vfs_desc);

    auto ioServiceDesc       = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name       = u8"CompilerRAMIOService";
    ioServiceDesc.sleep_time = 1000 / 60;
    project->ram_service     = skr_io_ram_service_t::create(&ioServiceDesc);
    project->ram_service->run();

    // Create output dir
    skr::filesystem::create_directories(project->GetOutputPath(), ec);

    return project;
}

bool SProject::LoadAssetData(skr::StringView uri, skr::Vector<uint8_t>& content) noexcept
{
    skr::String path       = uri;
    auto        asset_file = skr_vfs_fopen(asset_vfs, path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const auto  asset_size = skr_vfs_fsize(asset_file);
    content.resize_unsafe(asset_size);
    skr_vfs_fread(asset_file, content.data(), 0, asset_size);
    skr_vfs_fclose(asset_file);
    return true;
}

bool SProject::LoadAssetText(skr::StringView uri, skr::String& content) noexcept
{
    skr::String path       = uri;
    auto        asset_file = skr_vfs_fopen(asset_vfs, path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const auto  asset_size = skr_vfs_fsize(asset_file);
    content.append(u8'0', asset_size);
    skr_vfs_fread(asset_file, content.raw().data(), 0, asset_size);
    skr_vfs_fclose(asset_file);
    return true;
}

SProject::~SProject() noexcept
{
    if (ram_service) skr_io_ram_service_t::destroy(ram_service);
    if (resource_vfs) skr_free_vfs(resource_vfs);
    if (asset_vfs) skr_free_vfs(asset_vfs);
}
} // namespace skd
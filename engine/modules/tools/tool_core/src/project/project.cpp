#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/log.h"
#include "SkrCore/platform/vfs.h"
#include "SkrCore/serialize/json_archive.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrToolCore/project/project.hpp"

namespace skd
{

// void SProject::SetAssetVFS(skr_vfs_t* asset_vfs)
// {
//     this->asset_vfs = asset_vfs;
// }

// void SProject::SetResourceVFS(skr_vfs_t* resource_vfs)
// {
//     this->resource_vfs = resource_vfs;
// }

// void SProject::SetRAMService(skr::io::IRAMService* service)
// {
//     this->ram_service = service;
// }

skr::String SProject::GetEnv(skr::StringView name)
{
    auto iter = env.find(name);
    if (iter != env.end())
        return iter->second;
    return skr::String();
}

void SProject::SetEnv(skr::String k, skr::String v)
{
    env[k] = v;
}

skr_vfs_t* SProject::GetAssetVFS() const
{
    return asset_vfs;
}

skr_vfs_t* SProject::GetResourceVFS() const
{
    return resource_vfs;
}

skr_vfs_t* SProject::GetDependencyVFS() const
{
    return dependency_vfs;
}

skr::io::IRAMService* SProject::GetRamService() const
{
    return ram_service;
}

bool SProject::OpenProject(const skr::String& project_name, const skr::Path& root, const SProjectConfig& cfg) noexcept
{
    std::error_code ec = {};
    auto resolvePath = [&](const skr::String& path) -> skr::Path {
        skr::String result;
        result.reserve(path.size() + 256); // Reserve extra space for expansions

        auto view = path.view();
        auto currentView = view;

        while (!currentView.is_empty())
        {
            // Find next variable marker
            auto varStart = currentView.find(u8"${");
            if (!varStart)
            {
                // No more variables, append the rest
                result.append(currentView);
                break;
            }

            // Append text before variable
            result.append(currentView.subview(0, varStart.index()));

            // Find variable end
            auto remainingView = currentView.subview(varStart.index() + 2);
            auto varEnd = remainingView.find(u8"}");
            if (!varEnd)
            {
                // Unclosed variable, treat as literal text
                SKR_LOG_WARN(u8"Unclosed variable in path: %s", path.c_str());
                result.append(currentView.subview(varStart.index()));
                break;
            }

            // Extract and resolve variable name
            auto varName = remainingView.subview(0, varEnd.index());
            if (varName == u8"workspace")
            {
                auto workspace = skr::Path(GetEnv(u8"workspace"));
                result.append(workspace.string());
            }
            else if (varName == u8"platform")
            {
#if SKR_PLAT_WINDOWS
                result.append(u8"windows");
#elif SKR_PLAT_MACOSX
                result.append(u8"macosx");
#endif
            }
            else
            {
                // Unknown variable, keep as-is
                SKR_LOG_WARN(u8"Unknown variable '%s' in path: %s", skr::String(varName).c_str(), path.c_str());
                result.append(currentView.subview(varStart.index(), varEnd.index() + 3));
            }

            // Move to next part
            currentView = currentView.subview(varStart.index() + 2 + varEnd.index() + 1);
        }

        auto r = skr::Path(result);
        if (!r.is_absolute())
        {
            r = root / r;
        }
        return r;
    };

    name = project_name;

    assetDirectory = resolvePath(cfg.assetDirectory.string()).string();
    resourceDirectory = resolvePath(cfg.resourceDirectory.string()).string();
    artifactsDirectory = resolvePath(cfg.artifactsDirectory.string());
    dependencyDirectory = artifactsDirectory / u8"deps";

    // create resource VFS
    skr_vfs_desc_t resource_vfs_desc = {};
    resource_vfs_desc.app_name = name.u8_str();
    resource_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto u8outputPath = resourceDirectory;
    resource_vfs_desc.override_mount_dir = u8outputPath.string().c_str();
    resource_vfs = skr_create_vfs(&resource_vfs_desc);

    // create asset VFS
    skr_vfs_desc_t asset_vfs_desc = {};
    asset_vfs_desc.app_name = name.u8_str();
    asset_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto u8assetPath = assetDirectory;
    asset_vfs_desc.override_mount_dir = u8assetPath.string().c_str();
    asset_vfs = skr_create_vfs(&asset_vfs_desc);

    // create dependency VFS
    skr_vfs_desc_t dependency_vfs_desc = {};
    dependency_vfs_desc.app_name = name.u8_str();
    dependency_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    auto u8dependencyPath = dependencyDirectory;
    dependency_vfs_desc.override_mount_dir = u8dependencyPath.string().c_str();
    dependency_vfs = skr_create_vfs(&dependency_vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = u8"CompilerRAMIOService";
    ioServiceDesc.sleep_time = 1000 / 60;
    ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
    ram_service->run();

    // Create output directories
    skr_vfs_mkdir(resource_vfs, u8".");
    skr_vfs_mkdir(dependency_vfs, u8".");
    return true;
}

bool SProject::LoadAssetMeta(const URI& uri, skr::String& content) noexcept
{
    auto asset_file = skr_vfs_fopen(asset_vfs, uri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const auto asset_size = skr_vfs_fsize(asset_file);
    content.add(u8'0', asset_size);
    skr_vfs_fread(asset_file, content.data_raw_w(), 0, asset_size);
    skr_vfs_fclose(asset_file);
    return true;
}

bool SProject::SaveAssetMeta(const URI& uri, const skr::String& content) noexcept
{
    auto asset_file = skr_vfs_fopen(asset_vfs, uri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_ALWAYS_NEW);
    auto written = skr_vfs_fwrite(asset_file, content.data(), 0, content.size());
    SKR_ASSERT(written == content.size() && "Failed to write all bytes!");
    skr_vfs_fclose(asset_file);
    return true;
}

bool SProject::LoadAssetSourceFile(const URI& uri, skr::Vector<uint8_t>& content) noexcept
{
    auto asset_file = skr_vfs_fopen(asset_vfs, uri.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const auto asset_size = skr_vfs_fsize(asset_file);
    content.resize_unsafe(asset_size);
    skr_vfs_fread(asset_file, content.data(), 0, asset_size);
    skr_vfs_fclose(asset_file);
    return true;
}

bool SProject::OpenProject(const URI& projectFilePath) noexcept
{
    auto projectPath = skr::Path{ projectFilePath };
    skd::SProjectConfig cfg;
    {
        // Create a temporary VFS for reading the project file
        skr_vfs_desc_t temp_vfs_desc = {};
        temp_vfs_desc.app_name = u8"TempProjectLoader";
        temp_vfs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
        auto temp_vfs = skr_create_vfs(&temp_vfs_desc);

        auto projectFile = skr_vfs_fopen(temp_vfs, (const char8_t*)projectPath.string().c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        if (!projectFile)
        {
            SKR_LOG_ERROR(u8"Failed to open project file: %s", projectPath.string().c_str());
            skr_free_vfs(temp_vfs);
            return false;
        }

        // read string from file
        auto fileSize = skr_vfs_fsize(projectFile);
        skr::String projectFileContent;
        projectFileContent.add(u8'0', fileSize);
        skr_vfs_fread(projectFile, projectFileContent.data_raw_w(), 0, fileSize);
        skr_vfs_fclose(projectFile);
        skr_free_vfs(temp_vfs);

        // deserialize
        {
            auto reader = skr::ArReadJson::ReadBuffer(projectFileContent.data(), projectFileContent.size());
            reader.value(cfg);
            if (reader.is_failed())
            {
                SKR_LOG_ERROR(u8"Failed to parse project file: %s", projectPath.string().c_str());
                return false;
            }
        }
    }
    auto root = projectPath.parent_directory().string();
    auto name = projectPath.filename().string();
    return OpenProject(name.c_str(), root.c_str(), cfg);
}

bool SProject::CloseProject() noexcept
{
    return true;
}

SProject::~SProject() noexcept
{
    if (ram_service)
        skr_io_ram_service_t::destroy(ram_service);
    if (dependency_vfs)
        skr_free_vfs(dependency_vfs);
    if (resource_vfs)
        skr_free_vfs(resource_vfs);
    if (asset_vfs)
        skr_free_vfs(asset_vfs);
}
} // namespace skd
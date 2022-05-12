#include "gamert.h"
#include "platform/configure.h"
#include "ghc/filesystem.hpp"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"

void SGameRTModule::on_load()
{
    SKR_LOG_INFO("game runtime loaded!");
    auto resourceRoot = (ghc::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    skr::resource::SLocalResourceRegistry resourceRegistry(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(&resourceRegistry);
}

void SGameRTModule::main_module_exec()
{
    SKR_LOG_INFO("game runtime executed as main module!");
}

void SGameRTModule::on_unload()
{
    skr_free_vfs(resource_vfs);
    SKR_LOG_INFO("game runtime unloaded!");
}

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRT);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "GameRT",
    "prettyname" : "GameRuntime",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
GameRT)
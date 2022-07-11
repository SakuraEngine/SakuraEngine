#include "gamert.h"
#include "platform/configure.h"
#include "ghc/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"
#include "ecs/dual.h"
#include "runtime_module.h"

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRT);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "GameRT",
    "prettyname" : "GameRuntime",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRenderer", "version":"0.1.0"}, 
        {"name":"SkrGAInput", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
GameRT)

void SGameRTModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");
    auto resourceRoot = (ghc::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry);
}

void SGameRTModule::on_unload()
{
    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);
    skr_free_vfs(resource_vfs);

    SKR_LOG_INFO("game runtime unloaded!");
}
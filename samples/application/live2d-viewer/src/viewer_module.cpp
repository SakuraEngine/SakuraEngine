#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"

#include "ghc/filesystem.hpp"
#include "platform/vfs.h"
#include "utils/io.hpp"

#include "utils/make_zeroed.hpp"
#include "skr_live2d/model_resource.h"

#include "tracy/Tracy.hpp"

class SLive2DViewerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    skr_vfs_t* resource_vfs = nullptr;
    skr::io::RAMService* ram_service = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SLive2DViewerModule, Live2DViewer);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "Live2DViewer",
    "prettyname" : "Live2DViewer",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrLive2D", "version":"0.1.0"},
        {"name":"SkrImGui", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
Live2DViewer)

void SLive2DViewerModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer loaded!");

    auto resourceRoot = (ghc::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "Live2DViewerRAMIOService";
    ioServiceDesc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_IO_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);
}

void SLive2DViewerModule::on_unload()
{
    SKR_LOG_INFO("live2d viewer unloaded!");

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);
}

int SLive2DViewerModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer executed!");

    auto request = make_zeroed<skr_live2d_ram_io_request_t>();
    skr_io_ram_service_t* ioService = ram_service;
    request.vfs_override = resource_vfs;
    skr_live2d_model_create_from_json(ioService, "Live2DViewer/Haru/Haru.model3.json", &request);
    {
        ZoneScopedN("Idle");
        while(!request.is_ready());
    }
    skr_live2d_model_free(request.model_resource);
    return 0;
}
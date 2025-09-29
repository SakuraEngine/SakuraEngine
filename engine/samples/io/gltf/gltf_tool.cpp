/**
* use servide_io to load gltf files
*/
#include <SkrOS/filesystem.hpp>
#include <SkrCore/module/module_manager.hpp>
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/misc/cmd_parser.hpp"
#include "SkrMeshTool/gltf_processing.hpp"
#include "cgltf/cgltf.h"

// Module IOSample_gltf_tool

struct IOSampleGLTFToolModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;
};
IMPLEMENT_DYNAMIC_MODULE(IOSampleGLTFToolModule, IOSample_gltf_tool);
void IOSampleGLTFToolModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"IOSample_gltf_tool Module Loaded");
}
void IOSampleGLTFToolModule::on_unload()
{
    SKR_LOG_INFO(u8"IOSample_gltf_tool Module Unloaded");
}
int IOSampleGLTFToolModule::main_module_exec(int argc, char8_t** argv)
{
    SkrZoneScopedN("IOSample_gltf_tool::main_module_exec");
    SKR_LOG_INFO(u8"Running SkrMeshTool Module");
    skr::cmd::parser parser(argc, (char**)argv);
    parser.add(u8"gltf", u8"gltf file path", u8"-f", true);
    parser.add(u8"buffer", u8"load gltf buffer file", u8"-b", false);
    if (!parser.parse())
    {
        return 1;
    }
    auto gltf_path = parser.get_optional<skr::String>(u8"gltf");
    auto buffer_path = parser.get_optional<skr::String>(u8"buffer");
    if (!gltf_path)
    {
        SKR_LOG_ERROR(u8"gltf file path is required");
        return 1;
    }
    if (gltf_path->is_empty())
    {
        SKR_LOG_ERROR(u8"gltf file path is empty");
        return 1;
    }
    SKR_LOG_INFO(u8"gltf file path: {%s}", gltf_path->c_str());
    SKR_LOG_INFO(u8"buffer file path: {%s}", buffer_path ? buffer_path->c_str() : u8"none");

    // init vfs
    skr_vfs_t* abs_fs = nullptr;
    skr_vfs_desc_t abs_fs_desc = {};
    abs_fs_desc.app_name = u8"gltf_tool";
    abs_fs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    abs_fs = skr_create_vfs(&abs_fs_desc);

    SKR_LOG_INFO(u8"Absolute VFS mount dir: {%s}", abs_fs->mount_dir);

    skr_ram_io_service_desc_t ioServiceDesc = {};
    ioServiceDesc.name = u8"gltf_tool_io";
    ioServiceDesc.use_dstorage = false;
    ioServiceDesc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
    auto ioService = skr_io_ram_service_t::create(&ioServiceDesc);
    ioService->set_sleep_time(0); // make test faster
    ioService->run();
    SKR_LOG_INFO(u8"IO Service running");

    // Load file by blob directly
    // -----------------------------------
    // skr_io_future_t future = {};
    // skr::BlobId blob = nullptr;
    // {
    //     auto rq = ioService->open_request();
    //     rq->set_vfs(abs_fs);
    //     rq->set_path(gltf_path->c_str());
    //     rq->add_block({}); // read all
    //     blob = ioService->request(rq, &future);
    // }
    // wait_timeout([&future]()->bool
    // {
    //     return future.is_ready();
    // });
    // skr::String result = (const skr_char8*)blob->get_data();
    // SKR_LOG_INFO(u8"Loaded glTF file: {%s}", result.c_str());

    auto gltf_data = skr::ImportGLTFData(gltf_path->view(), ioService, abs_fs);
    // process binary buffer file automatically
    if (!gltf_data)
    {
        SKR_LOG_ERROR(u8"Failed to load glTF data");
        return 1;
    }
    SKR_LOG_INFO(u8"Successfully loaded glTF data");
    SKR_LOG_INFO(u8"Number of Nodes: %d", gltf_data->nodes_count);
    SKR_LOG_INFO(u8"Buffer Count: %d", gltf_data->buffers_count);
    if (gltf_data->buffers_count > 0)
    {
        SKR_LOG_INFO(u8"Buffer 0 Size: %zu bytes", gltf_data->buffers[0].size);
        SKR_LOG_INFO(u8"Buffer 0 Data: %p", gltf_data->buffers[0].data);
        // First 10 bytes of the first buffer
        if (gltf_data->buffers[0].data && gltf_data->buffers[0].size > 10)
        {
            SKR_LOG_INFO(u8"First 10 bytes of Buffer 0: ");
            for (size_t i = 0; i < 10; ++i)
            {
                SKR_LOG_INFO(u8"%02x ", ((uint8_t*)gltf_data->buffers[0].data)[i]);
            }
            SKR_LOG_INFO(u8"");
        }
    }

    // TODO: load to Mesh Resource

    skr_io_ram_service_t::destroy(ioService);

    return 0;
}

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    auto root = skr::fs::current_directory();
    {
        FrameMark;
        SkrZoneScopedN("Initialize");
        moduleManager->mount(root.string().c_str());
        moduleManager->make_module_graph(u8"IOSample_gltf_tool", true);
        moduleManager->init_module_graph(argc, argv);
        moduleManager->destroy_module_graph();
    }
    return 0;
}
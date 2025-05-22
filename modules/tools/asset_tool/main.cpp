#include "common/utils.h"
#include "SkrCore/time.h"
#include "SkrCore/platform/vfs.h"
#include <SkrContainers/string.hpp>
#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "SkrCore/module/module_manager.hpp"

#include <SkrImGui/imgui_backend.hpp>
#include <SkrImGui/imgui_render_backend.hpp>
#include <SkrImGui/imgui_utils.hpp>

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"

#include "SkrProfile/profile.h"

#include "SkrAssetTool/gltf_factory.h"
#include "nfd.h"
#include <algorithm>

class SAssetImportModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int  main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    // imgui
    skr::ImGuiBackend            imgui_backend;
    skr::ImGuiRendererBackendRG* render_backend_rg = nullptr;

public:
    static SAssetImportModule* Get();

    struct sugoi_storage_t* l2d_world    = nullptr;
    skr_vfs_t*              resource_vfs = nullptr;
};
#include "SkrOS/filesystem.hpp"

IMPLEMENT_DYNAMIC_MODULE(SAssetImportModule, SkrAssetImport);

SAssetImportModule* SAssetImportModule::Get()
{
    auto        mm = skr_get_module_manager();
    static auto rm = static_cast<SAssetImportModule*>(mm->get_module(u8"SkrAssetTool"));
    return rm;
}

void SAssetImportModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"live2d viewer loaded!");

    std::error_code ec           = {};
    auto            resourceRoot = (skr::filesystem::current_path(ec) / "../resources").u8string();
    skr_vfs_desc_t  vfs_desc     = {};
    vfs_desc.mount_type          = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir  = resourceRoot.c_str();
    resource_vfs                 = skr_create_vfs(&vfs_desc);
}

void SAssetImportModule::on_unload()
{
    SKR_LOG_INFO(u8"live2d viewer unloaded!");

    skr_free_vfs(resource_vfs);
}

int SAssetImportModule::main_module_exec(int argc, char8_t** argv)
{
    namespace render_graph = skr::render_graph;

    // configs
    skr::String                                filePath;
    skr::Vector<skd::asset::SImporterFactory*> factories;
    skr::Vector<skd::asset::SImporterFactory*> availableFactories;

    // get rendering context
    auto render_device = skr_get_default_render_device();
    auto cgpu_device   = render_device->get_cgpu_device();
    auto gfx_queue     = render_device->get_gfx_queue();
    auto renderGraph   = render_graph::RenderGraph::create(
        [=](skr::render_graph::RenderGraphBuilder& builder) {
            builder.with_device(cgpu_device)
                .with_gfx_queue(gfx_queue)
                .enable_memory_aliasing();
        }
    );

    // init imgui
    uint64_t frame_index = 0;
    {
        using namespace skr;
        auto render_backend = RCUnique<ImGuiRendererBackendRG>::New();
        render_backend_rg   = render_backend.get();
        ImGuiRendererBackendRGConfig config{};
        config.render_graph = renderGraph;
        config.queue        = gfx_queue;
        render_backend->init(config);
        imgui_backend.create({}, std::move(render_backend));
        imgui_backend.main_window().show();
        imgui_backend.enable_docking();
        // imgui_backend.enable_multi_viewport();
    }

    // timer
    SHiresTimer tick_timer;
    skr_init_hires_timer(&tick_timer);

    while (!imgui_backend.want_exit().comsume())
    {
        FrameMark;
        SkrZoneScopedN("LoopBody");

        // pump message
        {
            SkrZoneScopedN("PumpMessage");
            imgui_backend.pump_message();
        }

        // imgui new frame
        {
            SkrZoneScopedN("ImGUINewFrame");
            imgui_backend.begin_frame();
        }

        // imgui update
        {
            ImGui::Begin("Asset Importer");
            if (availableFactories.is_empty())
            {
                ImGui::InputText("File Path", &filePath);
                ImGui::SameLine();
                if (ImGui::Button("browse"))
                {
                    nfdchar_t*  outPath = nullptr;
                    nfdresult_t result  = NFD_OpenDialog("*", nullptr, &outPath);
                    if (result == NFD_OKAY)
                    {
                        filePath = (const char8_t*)outPath;
                        free(outPath);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Import"))
                {
                    for (auto factory : factories)
                        if (factory->CanImport(filePath))
                            availableFactories.add(factory);
                    if (availableFactories.is_empty())
                        SKR_LOG_ERROR(u8"No importer found for file: %s", filePath.c_str());
                    if (availableFactories.size() == 1)
                        if (availableFactories[0]->Import(filePath) != 0)
                        {
                            availableFactories.clear();
                            SKR_LOG_ERROR(u8"Failed to import file: %s", filePath.c_str());
                        }
                }
            }
            else if (availableFactories.size() > 1)
            {
                for (auto factory : availableFactories)
                {
                    if (ImGui::Button(factory->GetName().c_str_raw()))
                    {
                        if (factory->Import(filePath) == 0)
                        {
                            availableFactories.clear();
                            availableFactories.add(factory);
                            break;
                        }
                        else
                        {
                            availableFactories.remove(factory);
                            SKR_LOG_ERROR(u8"Failed to import file: %s", filePath.c_str());
                            break;
                        }
                    }
                }
            }
            else if (availableFactories.size() == 1)
            {
                if (availableFactories[0]->Update() != 0)
                    availableFactories.clear();
            }
            ImGui::End();
        }

        // imgui end frame
        {
            SkrZoneScopedN("ImGUIEndFrame");
            imgui_backend.end_frame();
        }

        // render imgui
        {
            SkrZoneScopedN("RenderImGui");
            imgui_backend.render();
        }

        // compile and execute render graph
        {
            SkrZoneScopedN("CompileRenderGraph");
            renderGraph->compile();
        }
        {
            SkrZoneScopedN("ExecuteRenderGraph");
            if (frame_index == 1000)
                render_graph::RenderGraphViz::write_graphviz(*renderGraph, "render_graph_L2D.gv");
            frame_index = renderGraph->execute();
            {
                SkrZoneScopedN("CollectGarbage");
                if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                    renderGraph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
            }
        }

        // do present
        {
            SkrZoneScopedN("Present");
            render_backend_rg->present_all();
        }
    }
    cgpu_wait_queue_idle(gfx_queue);
    render_graph::RenderGraph::destroy(renderGraph);
    imgui_backend.destroy();
    return 0;
}

int main(int argc, char** argv)
{
    auto            moduleManager = skr_get_module_manager();
    std::error_code ec            = {};
    auto            root          = skr::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph(u8"SkrAssetImport", true);
    auto result = moduleManager->init_module_graph(argc, argv);
    if (result != 0)
    {
        SKR_LOG_ERROR(u8"module graph init failed!");
    }
    moduleManager->destroy_module_graph();
    return 0;
}
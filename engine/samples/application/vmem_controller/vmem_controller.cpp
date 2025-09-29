#include "SkrProfile/profile.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrCore/module/module.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/imgui_app.hpp"
#if SKR_PLAT_WINDOWS
    #include <shellscalingapi.h>
#endif

class SVMemCCModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;
    void finalize();
    void imgui_ui();

    // Vulkan's memory heap is far more accurate than the one provided by D3D12
    ECGPUBackend backend = CGPU_BACKEND_D3D12;
    // ECGPUBackend backend = CGPU_BACKEND_D3D12;
    SRenderDeviceId render_device = nullptr;

    float vbuffer_size = 0.001f;
    float sbuffer_size = 0.001f;
    skr::Vector<CGPUBufferId> buffers;

    // imgui
    skr::UPtr<skr::ImGuiApp> imgui_app = nullptr;

    skr::RG::RenderGraph* graph = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SVMemCCModule, VMemController);

void SVMemCCModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"vmem controller loaded!");
    for (auto i = 0; i < argc; i++)
    {
        if (::strcmp((const char*)argv[i], "--vulkan") == 0)
        {
            backend = CGPU_BACKEND_VULKAN;
        }
        else if (::strcmp((const char*)argv[i], "--d3d12") == 0)
        {
            backend = CGPU_BACKEND_D3D12;
        }
    }
}

void SVMemCCModule::imgui_ui()
{
    ImGui::Begin("VideoMemoryController");
    SKR_UNUSED const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    SKR_UNUSED const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    auto adapter = render_device->get_cgpu_device()->adapter;
    auto device = render_device->get_cgpu_device();
    auto adapter_detail = cgpu_query_adapter_detail(adapter);
    // information
    {
        ImGui::Text("Card: %s", adapter_detail->vendor_preset.gpu_name);
        ImGui::Text("Driver: %d", adapter_detail->vendor_preset.driver_version);
    }
    // allocation
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    cgpu_query_video_memory_info(device, &total_bytes, &used_bytes);
    const auto total_mb = (float)total_bytes / 1024.f / 1024.f;
    const auto used_mb = (float)used_bytes / 1024.f / 1024.f;
    ImGui::Text("Used VMem: %.3f MB", used_mb);
    ImGui::SameLine();
    ImGui::Text("Usable VMem: %.3f MB", total_mb - used_mb);

    uint64_t total_shared_bytes = 0;
    uint64_t used_shared_bytes = 0;
    cgpu_query_shared_memory_info(device, &total_shared_bytes, &used_shared_bytes);
    const auto total_shared_mb = (float)total_shared_bytes / 1024.f / 1024.f;
    const auto used_shared_mb = (float)used_shared_bytes / 1024.f / 1024.f;
    ImGui::Text("Used SMem: %.3f MB", used_shared_mb);
    ImGui::SameLine();
    ImGui::Text("Usable SVMem: %.3f MB", total_shared_mb - used_shared_mb);

    ImGui::SliderFloat("##vbuffer", &vbuffer_size, 0.001f, total_mb - used_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button("AllocateVideoMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BUFFER_FLAG_NONE;
        buf_desc.usages = CGPU_BUFFER_USAGE_VERTEX_BUFFER;
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        buf_desc.size = (uint64_t)(vbuffer_size * 1024 * 1024);
        buf_desc.name = SKR_UTF8("VideoMemory");
        auto new_buf = cgpu_create_buffer(device, &buf_desc);
        buffers.add(new_buf);
    }

    ImGui::SliderFloat("##sbuffer", &sbuffer_size, 0.001f, total_shared_mb - used_shared_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button("AllocateSharedMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BUFFER_FLAG_NONE;
        buf_desc.usages = CGPU_BUFFER_USAGE_NONE;
        buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        buf_desc.size = (uint64_t)(sbuffer_size * 1024 * 1024);
        buf_desc.name = SKR_UTF8("SharedMemory");
        auto new_buf = cgpu_create_buffer(device, &buf_desc);
        buffers.add(new_buf);
    }

    // table
    const ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 10), 0.0f))
    {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, 0);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
        ImGui::TableSetupColumn("Size(MB)", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);
        ImGui::TableSetupColumn("Free", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, 3);
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();

        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin((int)buffers.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                // Display a data item
                auto buffer = buffers[row_n];
                ImGui::PushID(row_n);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%04d", row_n);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted((buffer->info->memory_usage == CGPU_MEM_USAGE_GPU_ONLY) ? "VideoMemory" : "SharedMemory");
                ImGui::TableNextColumn();
                ImGui::Text("%.3f", (float)buffer->info->size / 1024.f / 1024.f);
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Delete"))
                {
                    cgpu_free_buffer(buffer);
                    buffers[row_n] = nullptr;
                }
                ImGui::PopID();
            }
        ImGui::EndTable();
    }
    ImGui::End();
}

int SVMemCCModule::main_module_exec(int argc, char8_t** argv)
{
    SRenderDevice::Builder bd = {
        .backend = backend,
        .enable_debug_layer = false,
        .enable_gpu_based_validation = false,
        .enable_set_name = true
    };
    render_device = SRenderDevice::Create(bd);
    auto device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();

    // init imgui
    {
        using namespace skr;

        skr::RG::RenderGraphBuilder graph_builder;
        graph_builder.with_device(device)
            .with_gfx_queue(gfx_queue)
            .enable_memory_aliasing();

        imgui_app = UPtr<ImGuiApp>::New(
            SystemWindowCreateInfo{
                .title = u8"Video Memory Controller",
                .size = { 1280, 720 } },
            render_device, graph_builder);
        imgui_app->initialize();
        imgui_app->enable_docking();
    }

    // loop
    while (!imgui_app->want_exit().comsume())
    {
        FrameMark;
        static uint64_t frame_index = 0; // FIXME. bad usage

        // pump message
        imgui_app->pump_message();

        // imgui UI
        imgui_ui();

        // prepare rendering
        buffers.remove_all(nullptr);

        imgui_app->acquire_frames();
        imgui_app->set_load_action(CGPU_LOAD_ACTION_LOAD);

        // draw imgui
        imgui_app->render_imgui();

        // run rg
        frame_index = graph->execute();
        {
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }

        // present
        imgui_app->present_all();

        // Avoid too much CPU Usage
        skr_thread_sleep(16);
    }
    return 0;
}

void SVMemCCModule::on_unload()
{
    SKR_LOG_INFO(u8"vmem controller unloaded!");
    cgpu_wait_queue_idle(render_device->get_gfx_queue());
    imgui_app->shutdown();

    // clean up
    finalize();
}

void SVMemCCModule::finalize()
{
    // Free cgpu objects
    cgpu_wait_queue_idle(render_device->get_gfx_queue());
    for (auto buffer : buffers)
    {
        cgpu_free_buffer(buffer);
    }
    SRenderDevice::Destroy(render_device);
}

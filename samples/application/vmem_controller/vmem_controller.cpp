#include "common/utils.h"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/module/module.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/imgui_backend.hpp"
#include "SkrImGui/imgui_render_backend.hpp"
#if SKR_PLAT_WINDOWS
    #include <shellscalingapi.h>
#endif

#include "SkrProfile/profile.h"

class SVMemCCModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int  main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    void create_api_objects();
    void finalize();

    void imgui_ui();

    // Vulkan's memory heap is far more accurate than the one provided by D3D12
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    // ECGPUBackend backend = CGPU_BACKEND_D3D12;

    CGPUInstanceId instance       = nullptr;
    CGPUAdapterId  adapter        = nullptr;
    CGPUDeviceId   device         = nullptr;
    CGPUQueueId    gfx_queue      = nullptr;
    CGPUSamplerId  static_sampler = nullptr;

    float                     vbuffer_size = 0.001f;
    float                     sbuffer_size = 0.001f;
    skr::Vector<CGPUBufferId> buffers;

    // imgui
    skr::ImGuiBackend            imgui_backend;
    skr::ImGuiRendererBackendRG* imgui_render_backend = nullptr;

    skr::render_graph::RenderGraph* graph = nullptr;
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
    SKR_UNUSED const float TEXT_BASE_WIDTH  = ImGui::CalcTextSize("A").x;
    SKR_UNUSED const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    auto adapter_detail = cgpu_query_adapter_detail(adapter);
    // information
    {
        ImGui::Text("Card: %s", adapter_detail->vendor_preset.gpu_name);
        ImGui::Text("Driver: %d", adapter_detail->vendor_preset.driver_version);
    }
    // allocation
    uint64_t total_bytes = 0;
    uint64_t used_bytes  = 0;
    cgpu_query_video_memory_info(device, &total_bytes, &used_bytes);
    const auto total_mb = (float)total_bytes / 1024.f / 1024.f;
    const auto used_mb  = (float)used_bytes / 1024.f / 1024.f;
    ImGui::Text("Used VMem: %.3f MB", used_mb);
    ImGui::SameLine();
    ImGui::Text("Usable VMem: %.3f MB", total_mb - used_mb);

    uint64_t total_shared_bytes = 0;
    uint64_t used_shared_bytes  = 0;
    cgpu_query_shared_memory_info(device, &total_shared_bytes, &used_shared_bytes);
    const auto total_shared_mb = (float)total_shared_bytes / 1024.f / 1024.f;
    const auto used_shared_mb  = (float)used_shared_bytes / 1024.f / 1024.f;
    ImGui::Text("Used SMem: %.3f MB", used_shared_mb);
    ImGui::SameLine();
    ImGui::Text("Usable SVMem: %.3f MB", total_shared_mb - used_shared_mb);

    ImGui::SliderFloat("##vbuffer", &vbuffer_size, 0.001f, total_mb - used_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button("AllocateVideoMemory"))
    {
        auto buf_desc         = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags        = CGPU_BCF_NONE;
        buf_desc.descriptors  = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        buf_desc.size         = (uint64_t)(vbuffer_size * 1024 * 1024);
        buf_desc.name         = SKR_UTF8("VideoMemory");
        auto new_buf          = cgpu_create_buffer(device, &buf_desc);
        buffers.add(new_buf);
    }

    ImGui::SliderFloat("##sbuffer", &sbuffer_size, 0.001f, total_shared_mb - used_shared_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button("AllocateSharedMemory"))
    {
        auto buf_desc         = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags        = CGPU_BCF_NONE;
        buf_desc.descriptors  = CGPU_RESOURCE_TYPE_NONE;
        buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        buf_desc.size         = (uint64_t)(sbuffer_size * 1024 * 1024);
        buf_desc.name         = SKR_UTF8("SharedMemory");
        auto new_buf          = cgpu_create_buffer(device, &buf_desc);
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

#include "SkrRT/runtime_module.h"

int SVMemCCModule::main_module_exec(int argc, char8_t** argv)
{
    namespace render_graph = skr::render_graph;

    // init rendering
    {
        create_api_objects();

        graph = render_graph::RenderGraph::create(
            [=, this](skr::render_graph::RenderGraphBuilder& builder) {
                builder.with_device(device)
                    .with_gfx_queue(gfx_queue)
                    .enable_memory_aliasing();
            }
        );
    }

    // init imgui
    {
        using namespace skr;

        auto render_backend  = RCUnique<ImGuiRendererBackendRG>::New();
        imgui_render_backend = render_backend.get();
        ImGuiRendererBackendRGConfig config{};
        config.render_graph = graph;
        config.queue        = gfx_queue;
        render_backend->init(config);
        imgui_backend.create({}, std::move(render_backend));
        imgui_backend.main_window().show();
        imgui_backend.enable_docking();
    }

    // loop
    while (!imgui_backend.want_exit().comsume())
    {
        FrameMark;
        static uint64_t frame_index = 0; // FIXME. bad usage

        // pump message
        imgui_backend.pump_message();

        // imgui UI
        imgui_backend.begin_frame();
        imgui_ui();
        imgui_backend.end_frame();

        // prepare rendering
        buffers.remove_all(nullptr);

        // draw imgui
        imgui_backend.render();

        // run rg
        graph->compile();
        frame_index = graph->execute();
        {
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }

        // present
        imgui_render_backend->present_all();

        // Avoid too much CPU Usage
        skr_thread_sleep(16);
    }
    return 0;
}

void SVMemCCModule::on_unload()
{
    SKR_LOG_INFO(u8"vmem controller unloaded!");
    cgpu_wait_queue_idle(gfx_queue);
    skr::render_graph::RenderGraph::destroy(graph);
    imgui_backend.destroy();

    // clean up
    finalize();
}

void SVMemCCModule::create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc      = {};
    instance_desc.backend                     = backend;
    instance_desc.enable_debug_layer          = true;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name             = true;
    instance                                  = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queue_type               = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count              = 1;
    CGPUDeviceDescriptor device_desc          = {};
    device_desc.queue_groups                  = &queue_group_desc;
    device_desc.queue_group_count             = 1;
    device                                    = cgpu_create_device(adapter, &device_desc);
    gfx_queue                                 = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode           = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func          = CGPU_CMP_NEVER;
    static_sampler                     = cgpu_create_sampler(device, &sampler_desc);
}

void SVMemCCModule::finalize()
{
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    for (auto buffer : buffers)
    {
        cgpu_free_buffer(buffer);
    }
    cgpu_free_sampler(static_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

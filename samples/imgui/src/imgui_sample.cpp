#include "SkrGraphics/api.h"
#include "SkrImGui/imgui_render_backend.hpp"
#include <SkrImGui/imgui_backend.hpp>

int main()
{
    using namespace skr;

    // Create instance
    CGPUInstanceDescriptor instance_desc{};
    instance_desc.backend                     = CGPU_BACKEND_D3D12;
    instance_desc.enable_debug_layer          = true;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name             = true;
    auto instance                             = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    auto adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queue_type               = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count              = 1;
    CGPUDeviceDescriptor device_desc          = {};
    device_desc.queue_groups                  = &queue_group_desc;
    device_desc.queue_group_count             = 1;
    auto device                               = cgpu_create_device(adapter, &device_desc);
    auto gfx_queue                            = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    // create render graph
    auto render_graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(device)
                .with_gfx_queue(gfx_queue)
                .enable_memory_aliasing();
        }
    );

    // init imgui
    ImGuiBackend            imgui_backend;
    ImGuiRendererBackendRG* render_backend_rg = nullptr;
    {
        auto render_backend = RCUnique<ImGuiRendererBackendRG>::New();
        render_backend_rg   = render_backend.get();
        ImGuiRendererBackendRGConfig config{};
        config.render_graph = render_graph;
        config.queue        = gfx_queue;
        render_backend->init(config);
        imgui_backend.create({}, std::move(render_backend));
        imgui_backend.main_window().show();
        imgui_backend.enable_docking();
        imgui_backend.enable_high_dpi();
        // imgui_backend.enable_multi_viewport();
    }

    // draw loop
    bool     show_demo_window    = true;
    bool     show_another_window = true;
    ImVec4   clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    uint64_t frame_index         = 0;
    while (!imgui_backend.want_exit().comsume())
    {
        imgui_backend.pump_message();
        imgui_backend.begin_frame();

        {
            ImGuiIO& io = ImGui::GetIO();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f       = 0.0f;
                static int   counter = 0;

                ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }
        }

        imgui_backend.end_frame();

        // add render passes
        imgui_backend.render();

        // draw render graph
        render_graph->compile();
        render_graph->execute();
        if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
            render_graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);

        // present
        render_backend_rg->present_main_viewport();
        render_backend_rg->present_sub_viewports();
        ++frame_index;
    }

    // wait for rendering done
    cgpu_wait_queue_idle(gfx_queue);

    // destroy render graph
    render_graph::RenderGraph::destroy(render_graph);

    // destroy imgui
    imgui_backend.destroy();

    // shutdown cgpu
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
    return 0;
}
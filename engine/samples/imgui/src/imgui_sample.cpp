#include "SkrGraphics/api.h"
#include "SkrCore/memory/sp.hpp"
#include "SkrSystem/advanced_input.h"
#include <SkrImGui/imgui_app.hpp>

int main()
{
    using namespace skr;

    // initailize render device
    skr::RenderDevice::Builder builder = {};
    builder.enable_debug_layer = false;
    builder.enable_gpu_based_validation = false;
    builder.enable_set_name = true;
#ifdef _WIN32
    builder.backend = CGPU_BACKEND_D3D12;
#else
    builder.backend = CGPU_BACKEND_VULKAN;
#endif
    auto render_device = skr::RenderDevice::Create(builder);
    auto device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();

    // init imgui
    skr::UPtr<skr::ImGuiApp> imgui_app = nullptr;
    {
        RG::RenderGraphBuilder graph_builder;
        graph_builder.with_device(device)
            .with_gfx_queue(gfx_queue)
            .enable_memory_aliasing();

        skr::SystemWindowCreateInfo main_window_info = {
            .title = skr::format(u8"Live2D Viewer Inner [{}]", gCGPUBackendNames[device->adapter->instance->backend]),
            .size = { 1500, 1500 },
        };

        imgui_app = skr::UPtr<skr::ImGuiApp>::New(main_window_info, render_device, graph_builder);
        imgui_app->initialize();
        imgui_app->enable_docking();
        // imgui_backend.enable_multi_viewport();
    }
    auto render_graph = imgui_app->render_graph();

    // draw loop
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    uint64_t frame_index = 0;
    skr::input::Input::Initialize();

    while (!imgui_app->want_exit().comsume())
    {
        imgui_app->pump_message();
        {
            ImGuiIO& io = ImGui::GetIO();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

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

        imgui_app->acquire_frames();
        imgui_app->render_imgui();

        frame_index = render_graph->execute();
        if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
            render_graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);

        // present
        imgui_app->present_all();
    }

    // wait for rendering done
    cgpu_wait_queue_idle(gfx_queue);

    // destroy imgui
    imgui_app->shutdown();

    // shutdown cgpu
    skr::RenderDevice::Destroy(render_device);
    return 0;
}
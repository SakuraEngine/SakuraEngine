#include "elem_application.h"
#include "gui_render_graph.hpp"

#include "containers/text.hpp"

#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrGui/framework/window_context.hpp"

#include "SkrGui/render_elements/render_window.hpp"
#include "SkrGui/render_elements/render_canvas.hpp"
#include "SkrGui/render_elements/render_grid_paper.hpp"

#include "SkrGuiRenderer/gdi_renderer.hpp"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#ifdef SKR_OS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")
#endif

extern void create_imgui_resources(ECGPUFormat format, CGPUSamplerId sampler, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);

struct elements_example_application : public elements_application_t
{
    CGPUSamplerId imgui_sampler = nullptr;
    bool initialize()
    {
#ifdef SKR_OS_WINDOWS
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif

        // initialize base app
        if (!initialize_elem_application(this)) return false;

        // add elements
        canvas = SkrNew<skr::gui::RenderCanvas>(gdi.device);
        grid_paper = SkrNew<skr::gui::RenderGridPaper>(gdi.device);
        root_window->add_child(canvas);
        canvas->add_child(grid_paper);

        // initialize render graph
        if (graph.initialize(gdi.gfx))
        {
            CGPUSamplerDescriptor sampler_desc = {};
            sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
            sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
            sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
            sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_NEAREST;
            sampler_desc.min_filter = CGPU_FILTER_TYPE_NEAREST;
            sampler_desc.mag_filter = CGPU_FILTER_TYPE_NEAREST;
            sampler_desc.compare_func = CGPU_CMP_NEVER;
            imgui_sampler = cgpu_create_sampler(gdi.gfx.device, &sampler_desc);
            const auto backbuffer_format = (ECGPUFormat)gdi.gfx.swapchain->back_buffers[0]->format;
            create_imgui_resources(backbuffer_format, imgui_sampler, graph.graph, gdi.resource_vfs);
            return true;
        }
        return false;
    }

    const skr::gui::DiagnosticableTreeNode* selected_diagnostic = nullptr;
    void diagnostics_inspect_recursively(const skr::gui::DiagnosticableTreeNode* diagnostic)
    {
        ImGui::PushID(diagnostic);
        auto type_property = static_cast<skr::gui::TextDiagnosticProperty*>(diagnostic->find_property("type"));
        auto type = type_property ? type_property->get_value() : "element";
        skr::text::text show_name = skr::text::format("{}{}{}", "[", type, "]");
        ImGuiTreeNodeFlags node_flags = (selected_diagnostic == diagnostic) ? ImGuiTreeNodeFlags_Selected : 0;
        node_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        if (ImGui::TreeNodeEx(show_name.c_str(), node_flags))
		{
            if (ImGui::IsItemClicked()) 
            {
                selected_diagnostic = diagnostic;
            }
			for (const auto diagnostic_node : diagnostic->get_diagnostics_children())
            {
				diagnostics_inspect_recursively(diagnostic_node);
            }
			ImGui::TreePop();
		}
        ImGui::PopID();
    }

    void diagnostics_inspect()
    {
        ZoneScopedN("ImGUINewFrame");

        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)gdi.gfx.swapchain->back_buffers[0]->width, (float)gdi.gfx.swapchain->back_buffers[0]->height);
        skr_imgui_new_frame(gdi.gfx.window_handle, 1.f / 60.f);

        ImGui::Begin("GUI RenderElements Example");
        ImGui::Columns(2, "DockSpace");
        {
            ImGui::BeginChild("TreeView");

            diagnostics_inspect_recursively(root_window);
            ImGui::EndChild();
        }
        ImGui::NextColumn(); 
        {
            ImGui::BeginChild("Properties");
            if (selected_diagnostic)
            {
                for (const auto property : selected_diagnostic->get_diagnostics_properties())
                {
                    ImGui::Text(property->get_name());
                    ImGui::SameLine();
                    ImGui::Text(property->get_value_as_string());
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void draw()
    {
        // VG
        skr::gui::WindowContext::DrawParams draw_params = {};
        window_context->draw(&draw_params);
    }

    void render()
    {
        // declare render resources
        graph.declare_render_resources(gdi.gfx);

        // render
        skr::gui::WindowContext::RenderParams render_params = {};
        skr::gdi::ViewportRenderParams vp_render_params = {};
        skr::gdi::ViewportRenderParams_RenderGraph vp_render_params2 = {};
        vp_render_params2.render_graph = graph.graph;
        vp_render_params.usr_data = &vp_render_params2;
        render_params.gdi_params = &vp_render_params;
        window_context->render(gdi.renderer, &render_params);

        // imgui pass
        {
            ZoneScopedN("RenderIMGUI");
            render_graph_imgui_add_render_pass(graph.graph, graph.back_buffer, CGPU_LOAD_ACTION_LOAD);
        }

        // submit graph
        graph.submit_render_graph(gdi.gfx);
        render_graph_imgui_present_sub_viewports();
    }

    void finalize()
    {
        // clean up
        app_wait_gpu_idle(&gdi.gfx);

        graph.finalize();

        if (imgui_sampler) cgpu_free_sampler(imgui_sampler);
        render_graph_imgui_finalize();
        
        // free render elements
        SkrDelete(grid_paper);
        SkrDelete(canvas);
        
        // free base app
        finalize_elem_application(this);
    }

    skr::gui::RenderCanvas* canvas = nullptr;
    skr::gui::RenderGridPaper* grid_paper = nullptr;
    gui_render_graph_t graph;
};

#include "tracy/Tracy.hpp"

int main(int argc, char* argv[])
{
    auto App = make_zeroed<elements_example_application>();
    App.initialize();
    bool quit = false;
    while (!quit)
    {
        FrameMark;
        {
            ZoneScopedN("SystemEvents");
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                auto sdl_window = (SDL_Window*)App.gdi.gfx.window_handle;
                if (SDL_GetWindowID(sdl_window) == event.window.windowID)
                {
                    if (!SDLEventHandler(&event, sdl_window))
                    {
                        quit = true;
                    }
                }

                if (event.type == SDL_WINDOWEVENT)
                {
                    Uint8 window_event = event.window.event;
                    if (window_event == SDL_WINDOWEVENT_CLOSE || window_event == SDL_WINDOWEVENT_MOVED || window_event == SDL_WINDOWEVENT_RESIZED)
                    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(event.window.windowID)))
                    {
                        if (window_event == SDL_WINDOWEVENT_CLOSE)
                            viewport->PlatformRequestClose = true;
                        if (window_event == SDL_WINDOWEVENT_MOVED)
                            viewport->PlatformRequestMove = true;
                        if (window_event == SDL_WINDOWEVENT_RESIZED)
                            viewport->PlatformRequestResize = true;
                    }
                }

                if (event.type == SDL_WINDOWEVENT)
                {
                    uint8_t window_event = event.window.event;
                    if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        app_resize_window(&App.gdi.gfx, event.window.data1, event.window.data2);
                    }
                }
            }
        }
        {
            ZoneScopedN("DiagnosticsInspect");
            App.diagnostics_inspect();
        }
        {
            ZoneScopedN("DrawGUI");
            App.draw();
        }
        {
            ZoneScopedN("RenderGUI");
            App.render();
        }
    }
    App.finalize();
    return 0;
}
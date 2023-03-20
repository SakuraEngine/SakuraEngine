#include "elem_application.h"
#include "gui_render_graph.hpp"

#include "containers/text.hpp"

#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrGui/framework/window_context.hpp"

#include "SkrGui/render_elements/render_window.hpp"
#include "SkrGui/render_elements/render_canvas.hpp"
#include "SkrGui/render_elements/render_grid_paper.hpp"
#include "SkrGui/render_elements/render_color_picker.hpp"
#include "SkrGui/render_elements/render_flex.hpp"
#include "SkrGui/render_elements/render_image.hpp"
#include "SkrGui/render_elements/render_text.hpp"

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

#include "SkrInput/input.h"

extern void create_imgui_resources(ECGPUFormat format, CGPUSamplerId sampler, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);

struct elements_example_application : public elements_application_t
{
    CGPUSamplerId imgui_sampler = nullptr;
    bool initialize()
    {
#ifdef SKR_OS_WINDOWS
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif

        skr::input::Input::Initialize();

        // initialize base app
        if (!initialize_elem_application(this)) return false;

        // add elements
        canvas = SkrNew<skr::gui::RenderCanvas>(gdi.device);
        grid_paper = SkrNew<skr::gui::RenderGridPaper>(gdi.device);
        color_picker = SkrNew<skr::gui::RenderColorPicker>(gdi.device);
        flex = SkrNew<skr::gui::RenderFlex>(gdi.device);
        flex->set_align_items(skr::gui::AlignItems::Center);
        flex->set_justify_content(skr::gui::JustifyContent::SpaceAround);
        image1 = SkrNew<skr::gui::RenderImage>(gdi.device);
        image1->set_color({ 1, 0, 0, 1 });
        image2 = SkrNew<skr::gui::RenderImage>(gdi.device);
        image2->set_color({ 0, 1, 0, 1 });
        image3 = SkrNew<skr::gui::RenderImage>(gdi.device);
        image3->set_color({ 0, 0, 1, 1 });
        // text_on_image3 = SkrNew<skr::gui::RenderText>(gdi.device);
        // text_on_image3->add_text("Hello World!");
        
        root_window->add_child(canvas);
        canvas->add_child(grid_paper);
        canvas->add_child(color_picker);
        flex->add_child(image1);
        image1->set_size({ 100, 300 });
        flex->add_child(image2);
        image2->set_size({ 100, 200 });
        flex->add_child(image3);
        image3->set_size({ 100, 400 });
        canvas->add_child(flex);
        // text_on_image3->set_size({ 50, 200 });
        // flex->add_child(text_on_image3);

        flex->layout(skr::gui::BoxConstraint{{400, 400}, {0, 0}}, true);

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

    skr::gui::DiagnosticableTreeNode* selected_diagnostic = nullptr;
    void diagnostics_inspect_recursively(skr::gui::DiagnosticableTreeNode* diagnostic)
    {
        ImGui::PushID(diagnostic);
        auto type_property = static_cast<skr::gui::TextDiagnosticProperty*>(diagnostic->find_property("type"));
        auto type = type_property ? type_property->get_value() : "element";
        skr::text::text show_name = skr::text::format("{}{}{}", "[", type, "]");
        ImGuiTreeNodeFlags node_flags = (selected_diagnostic == diagnostic) ? ImGuiTreeNodeFlags_Selected : 0;
        node_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
        if (diagnostic->get_diagnostics_children().empty())
        {
            node_flags |= ImGuiTreeNodeFlags_Leaf;
        }
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
        auto diagnostic_as_render_box = [&](){
            if (selected_diagnostic)
            {
                if (auto prop = selected_diagnostic->find_property("render_box"))
                {
                    auto& bProp = prop->as<skr::gui::BoolDiagnosticProperty>();
                    if (bProp.value && bProp.value.get())
                    {
                        auto render_box = static_cast<skr::gui::RenderBox*>(selected_diagnostic);
                        return render_box;
                    }
                }
            }
            return (skr::gui::RenderBox*)nullptr;
        };

        auto render_box = diagnostic_as_render_box();
        if(render_box) render_box->enable_debug_draw(true);

        skr::gui::WindowContext::DrawParams draw_params = {};
        window_context->draw(&draw_params);
        
        if(render_box) render_box->enable_debug_draw(false);
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
        // SkrDelete(text_on_image3);
        SkrDelete(image1);
        SkrDelete(image2);
        SkrDelete(image3);
        SkrDelete(flex);
        SkrDelete(color_picker);
        SkrDelete(grid_paper);
        SkrDelete(canvas);
        
        // free base app
        finalize_elem_application(this);

        skr::input::Input::Finalize();
    }

    skr::gui::RenderCanvas* canvas = nullptr;
    skr::gui::RenderGridPaper* grid_paper = nullptr;
    skr::gui::RenderColorPicker* color_picker = nullptr;
    skr::gui::RenderFlex* flex = nullptr;
    skr::gui::RenderImage* image1 = nullptr;
    skr::gui::RenderImage* image2 = nullptr;
    skr::gui::RenderImage* image3 = nullptr;
    // skr::gui::RenderText* text_on_image3 = nullptr;
    gui_render_graph_t graph;
};

#include "tracy/Tracy.hpp"
#include "utils/log.h"
#include "utils/defer.hpp"

struct KeyboardTest
{
    skr::input::InputLayer* pLayer = nullptr;
    void PollKeyboardInput() noexcept 
    {
        using namespace skr::input;
        if (auto input = skr::input::Input::GetInstance())
        {
            InputReading* pReading = nullptr;
            if (INPUT_RESULT_OK == input->GetCurrentReading(InputKindKeyboard, nullptr, &pLayer, &pReading))
            {
                const auto currentTimestamp = pLayer->GetCurrentTimestampUSec();

                InputKeyState keystates[16];
                uint32_t readCount = pLayer->GetKeyState(pReading, 16, keystates);
                const auto timestamp = pLayer->GetTimestampUSec(pReading);
                const auto elapsed_us = currentTimestamp - timestamp;
                for (uint32_t j = 0; j < readCount; j++)
                {
                    auto k = keystates[j];
                    SKR_LOG_INFO("GameInput: Key:0x%02X, Timestamp: %lld, Elapsed: %d us(%d ms), Dead:%d",
                        keystates[j].virtual_key, timestamp, elapsed_us, elapsed_us / 1000, k.is_dead_key);
                }
                if (pReading) pLayer->Release(pReading);
            }
        }
    }
    ~KeyboardTest()
    {

    }
};

struct ClickListener
{
    ClickListener(uint32_t threshold_in_ms = 500) : ThresholdInMs(threshold_in_ms) {}
    // ~ClickListener() { if (Mouse) Mouse->Release(); if (previous) previous->Release(); }
    skr::input::InputLayer* pLayer = nullptr;
    skr::input::InputDevice* Mouse = nullptr; 
    skr::input::InputReading* previous = nullptr; 
    bool WasUp = false;
    uint32_t Counter = 0;
    uint32_t ThresholdInMs = 0;
    bool isDown(const skr::input::InputMouseState& state) 
    { 
        return (state.buttons & skr::input::InputMouseLeftButton) && (state.buttons & skr::input::InputMouseRightButton); 
    }

    uint32_t Trigger()
    {
        using namespace skr::input;
        InputReading* current = nullptr; 
        if (auto input = skr::input::Input::GetInstance())
        {
            if (input->GetCurrentReading(InputKindMouse, Mouse, &pLayer, &current) == INPUT_RESULT_OK) 
            { 
                InputMouseState mouseState = {};
                if (pLayer->GetMouseState(current, &mouseState)) 
                { 
                    SKR_DEFER({WasUp = !isDown(mouseState);});
                    if (isDown(mouseState) && previous != current && WasUp)
                    {
                        SKR_DEFER({ if (previous) pLayer->Release(previous); previous = current; });
                        if (previous && pLayer->GetTimestampUSec(previous) + ThresholdInMs * 1000.f >= pLayer->GetTimestampUSec(current))
                        {
                            Counter++;
                        } 
                        else /* new click */ {
                            if (Mouse) pLayer->Release(Mouse);
                            pLayer->GetDevice(current, &Mouse); 
                            Counter = 1;
                        }
                        return Counter;
                    }
                }
                pLayer->Release(current);
            } 
        }
        return 0;
    }

    void PollMouseInput() noexcept 
    { 
        if (uint32_t trigger_count = Trigger())
        {
            if (trigger_count)
            {
                SKR_LOG_INFO("Clicked %d times", trigger_count);
            }
        }
    }
};

#include <iostream>
void UpdateScan(skr::span<uint8_t> write_span)
{
    int numkeys;
    const uint8_t* state = SDL_GetKeyboardState(&numkeys);
    for (int scancode = 0, i = 0; scancode < numkeys && i < write_span.size(); ++scancode)
    {
        if (state[scancode])
            write_span[i++] = scancode;
    }
}

int main(int argc, char* argv[])
{
    auto App = make_zeroed<elements_example_application>();
    App.initialize();
    bool quit = false;
    KeyboardTest keyboard_test;
    ClickListener doubleClickListener = ClickListener(500);
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

                if (event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
                {
                    int mouse_button = -1;
                    if (event.button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
                    if (event.button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
                    if (event.button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
                    if (event.button.button == SDL_BUTTON_X1) { mouse_button = 3; }
                    if (event.button.button == SDL_BUTTON_X2) { mouse_button = 4; }

                    ImGuiIO& io = ImGui::GetIO();
                    io.AddMouseButtonEvent(mouse_button, (event.type == SDL_MOUSEBUTTONDOWN));
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
            skr::vector<uint8_t> scan_codes(16);
            UpdateScan(scan_codes);
            for (auto code : scan_codes) {
                if (code != 0) {
                    std::cout << "Scan code: " << static_cast<int>(code) << std::endl;
                }
            }
        }
        {
            skr::input::Input::GetInstance()->Tick();
            keyboard_test.PollKeyboardInput();
            doubleClickListener.PollMouseInput();
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
#include "robjects_application.h"
#include "gui_render_graph.hpp"

#include "SkrContainers/string.hpp"

#include "SkrCore/platform/system.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/defer.hpp"

#include "SkrInputSystem/input_system.hpp"

#include "SkrGui/dev/gdi/gdi.hpp"
#include "SkrGui/dev/window_context.hpp"

#include "SkrGui/framework/widget_misc.hpp"
#include "SkrGui/render_objects/render_grid_paper.hpp"
#include "SkrGui/render_objects/render_color_picker.hpp"
#include "SkrGui/render_objects/render_text.hpp"

#include "SkrGuiRenderer/gdi_renderer.hpp"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#if SKR_PLAT_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <shellscalingapi.h>
    #pragma comment(lib, "Shcore.lib")
#endif

#include "SkrInput/input.h"

#include "SkrProfile/profile.h"

extern void create_imgui_resources(ECGPUFormat format, CGPUSamplerId sampler, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);

struct robjects_example_application : public robjects_application_t {
    CGPUSamplerId imgui_sampler = nullptr;
    bool          initialize()
    {
#if SKR_PLAT_WINDOWS
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
        // initialize base app
        if (!initialize_robjects_application(this)) return false;

        skr::input::Input::Initialize();
        input_system = skr::input::InputSystem::Create();
        auto mapping_ctx = input_system->create_mapping_context();
        input_system->add_mapping_context(mapping_ctx, 0, {});
        auto mapping = input_system->create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_F);
        auto action = input_system->create_input_action(skr::input::EValueType::kBool);
        auto trigger = input_system->create_trigger<skr::input::InputTriggerDown>();
        action->add_trigger(trigger);
        action->bind_event<bool>([](const bool& down) {
            SKR_LOG_INFO(u8"Key F pressed: %d", down);
        });
        mapping->action = action;
        mapping_ctx->add_mapping(mapping);

        auto mapping2 = input_system->create_mapping<skr::input::InputMapping_MouseButton>(EMouseKey::MOUSE_KEY_LB);
        auto action2 = input_system->create_input_action(skr::input::EValueType::kBool);
        auto trigger2 = input_system->create_trigger<skr::input::InputTriggerPressed>();
        action2->add_trigger(trigger2);
        action2->bind_event<bool>([](const bool& f2) {
            int x, y;
            skr_cursor_pos(&x, &y, ECursorCoordinate::CURSOR_COORDINATE_WINDOW);
            SKR_LOG_INFO(u8"Mouse Clicked at: X[%d] Y[%d]", x, y);
        });
        mapping2->action = action2;
        mapping_ctx->add_mapping(mapping2);

        // TODO. init sandbox

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
            const auto backbuffer_format = (ECGPUFormat)gdi.gfx.swapchain->back_buffers[0]->info->format;
            create_imgui_resources(backbuffer_format, imgui_sampler, graph.graph, gdi.resource_vfs);
            return true;
        }
        return false;
    }

    // TODO. diagnostic

    void tick(float delta)
    {
        skr::input::Input::GetInstance()->Tick();
        input_system->update(delta);
    }

    void draw()
    {
        // TODO. draw diagnostic
        // TODO. draw sandbox
    }

    void render()
    {
        // declare render resources
        graph.declare_render_resources(gdi.gfx);

        // render
        skr::gui::WindowContext::RenderParams      render_params = {};
        skr::gui::ViewportRenderParams             vp_render_params = {};
        skr::gui::ViewportRenderParams_RenderGraph vp_render_params2 = {};
        vp_render_params2.render_graph = graph.graph;
        vp_render_params.usr_data = &vp_render_params2;
        render_params.gdi_params = &vp_render_params;
        window_context->render(gdi.renderer, &render_params);

        // imgui pass
        {
            SkrZoneScopedN("RenderIMGUI");
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

        // TODO. free sandbox

        skr::input::InputSystem::Destroy(input_system);
        skr::input::Input::Finalize();

        // free base app
        finalize_robjects_application(this);
    }

    gui_render_graph_t graph;

    skr::input::InputSystem* input_system = nullptr;
};

struct KeyboardTest {
    skr::input::InputLayer* pLayer = nullptr;
    void                    PollKeyboardInput() noexcept
    {
        using namespace skr::input;
        if (auto input = skr::input::Input::GetInstance())
        {
            InputReading* pReading = nullptr;
            if (INPUT_RESULT_OK == input->GetCurrentReading(InputKindKeyboard, nullptr, &pLayer, &pReading))
            {
                const auto currentTimestamp = pLayer->GetCurrentTimestampUSec();

                InputKeyState keystates[16];
                uint32_t      readCount = pLayer->GetKeyState(pReading, 16, keystates);
                const auto    timestamp = pLayer->GetTimestampUSec(pReading);
                const auto    elapsed_us = currentTimestamp - timestamp;
                for (uint32_t j = 0; j < readCount; j++)
                {
                    auto k = keystates[j];
                    SKR_LOG_INFO(u8"GameInput: Key:0x%02X, Timestamp: %lld, Elapsed: %d us(%d ms), Dead:%d",
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

struct ClickListener {
    ClickListener(uint32_t threshold_in_ms = 500)
        : ThresholdInMs(threshold_in_ms)
    {
    }
    // ~ClickListener() { if (Mouse) Mouse->Release(); if (previous) previous->Release(); }
    skr::input::InputLayer*   pLayer = nullptr;
    skr::input::InputDevice*  Mouse = nullptr;
    skr::input::InputReading* previous = nullptr;
    bool                      WasUp = false;
    uint32_t                  Counter = 0;
    uint32_t                  ThresholdInMs = 0;
    bool                      isDown(const skr::input::InputMouseState& state)
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
                    SKR_DEFER({ WasUp = !isDown(mouseState); });
                    if (isDown(mouseState) && previous != current && WasUp)
                    {
                        SKR_DEFER({ if (previous) pLayer->Release(previous); previous = current; });
                        if (previous && pLayer->GetTimestampUSec(previous) + ThresholdInMs * 1000.f >= pLayer->GetTimestampUSec(current))
                        {
                            Counter++;
                        }
                        else /* new click */
                        {
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
                SKR_LOG_INFO(u8"Clicked %d times", trigger_count);
            }
        }
    }
};

#include <iostream>
void UpdateScan(skr::span<uint8_t> write_span)
{
    int            numkeys;
    const uint8_t* state = SDL_GetKeyboardState(&numkeys);
    for (int scancode = 0, i = 0; scancode < numkeys && i < write_span.size(); ++scancode)
    {
        if (state[scancode])
            write_span[i++] = scancode;
    }
}

int main(int argc, char* argv[])
{
    SkrDStorageConfig config = {};
    skr_create_dstorage_instance(&config);
    auto App = make_zeroed<robjects_example_application>();
    App.initialize();
    bool          quit = false;
    KeyboardTest  keyboard_test;
    ClickListener doubleClickListener = ClickListener(500);
    auto          handler = skr_system_get_default_handler();
    handler->add_window_close_handler(
    +[](SWindowHandle window, void* pQuit) {
        bool& quit = *(bool*)pQuit;
        quit = true;
    },
    &quit);
    handler->add_window_resize_handler(
    +[](SWindowHandle window, int32_t w, int32_t h, void* usr_data) {
        robjects_example_application* pApp = (robjects_example_application*)usr_data;
        app_resize_window(&pApp->gdi.gfx, w, h);
    },
    &App);
    skr_imgui_initialize(handler);
    while (!quit)
    {
        FrameMark;
        float delta = 1.f / 60.f;
        {
            SkrZoneScopedN("SystemEvents");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }
        {
            App.tick(delta);
        }
        {
            keyboard_test.PollKeyboardInput();
            doubleClickListener.PollMouseInput();
        }
        {
            SkrZoneScopedN("DrawGUI");
            App.draw();
        }
        {
            SkrZoneScopedN("RenderGUI");
            App.render();
        }
    }
    App.finalize();
    return 0;
}
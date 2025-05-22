#include <chrono>
#include "SkrGui/system/input/hit_test.hpp"
#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/dev/sandbox.hpp"
#include "SkrProfile/profile.h"
#include "SkrGui/backend/device/window.hpp"
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_trigger.hpp"
#include "input_binding.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

#include "counter_state.hpp"
#include <SkrGui/backend/device/window.hpp>
#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGuiRenderer/device/skr_native_window.hpp"

// !!!! TestWidgets !!!!
#include "SkrGui/widgets/stack.hpp"
#include "SkrGui/widgets/color_picker.hpp"
#include "SkrGui/widgets/colored_box.hpp"
#include "SkrGui/widgets/flex.hpp"
#include "SkrGui/widgets/grid_paper.hpp"
#include "SkrGui/widgets/positioned.hpp"
#include "SkrGui/widgets/sized_box.hpp"
#include "SkrGui/widgets/text.hpp"
#include "SkrGui/widgets/flex_slot.hpp"
#include "SkrGui/widgets/mouse_region.hpp"

int main(void)
{
    using namespace skr::gui;

    // log system
    skr::logging::LogConstants::gFlushBehavior = skr::logging::LogFlushBehavior::kFlushImmediate;

    // create backends
    SkrNativeDevice* device = SkrNew<SkrNativeDevice>();
    device->init();

    // create sandbox
    Sandbox* sandbox = SkrNew<Sandbox>(device);
    sandbox->init();

    // setup content
    {
        auto widget = SNewWidget(Counter){};
        sandbox->set_content(widget);
    }

    // input system
    skr::input::InputSystem* input_system = nullptr;
    {
        // init input system
        skr::input::Input::Initialize();
        input_system = skr::input::InputSystem::Create();

        // create mapping context
        auto mapping_ctx = input_system->create_mapping_context();
        input_system->add_mapping_context(mapping_ctx, 0, {});

        // bind events
        bind_pointer_event(input_system, mapping_ctx, sandbox);
    }

    // handler
    bool b_quit = false;

    // show window
    {
        WindowDesc desc = {};
        desc.size       = { 600, 600 };
        desc.pos        = { 300, 300 };
        desc.name       = u8"SkrGUI Sandbox";
        sandbox->show(desc);
    }

    // run application
    auto prev_time    = std::chrono::high_resolution_clock::now();
    auto startup_time = std::chrono::high_resolution_clock::now();
    while (!b_quit)
    {
        // update time
        auto cur_time       = std::chrono::high_resolution_clock::now();
        auto delta          = cur_time - prev_time;
        prev_time           = cur_time;
        auto delta_sec      = std::chrono::duration<double>(delta).count();
        auto time_stamp     = cur_time - startup_time;
        auto time_stamp_sec = std::chrono::duration<double>(time_stamp).count();

        FrameMark;
        {
            SkrZoneScopedN("SystemEvents");
            SDL_Event e;
            auto      sdl_window = sandbox->native_window()->window()->type_cast_fast<SkrNativeWindow>()->window();
            while (SDL_PollEvent(&e))
            {
                switch (e.type)
                {
                case SDL_EVENT_QUIT:
                    b_quit = true;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    if (e.window.windowID == SDL_GetWindowID(sdl_window))
                    {
                        b_quit = true;
                    }
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    if (e.window.windowID == SDL_GetWindowID(sdl_window))
                    {
                        sandbox->resize_window(e.window.data1, e.window.data2);
                    }
                    break;
                }
            }
        }
        {
            SkrZoneScopedN("InputSystem");
            skr::input::Input::GetInstance()->Tick();
            input_system->update(delta_sec);
        }
        {
            SkrZoneScopedN("Sandbox");
            sandbox->update(time_stamp_sec);
            sandbox->layout();
            sandbox->paint();
            sandbox->compose();
        }
        {
            device->render_all_windows();
        }
    }

    // finalize
    device->shutdown();
    SkrDelete(device);

    return 0;
}
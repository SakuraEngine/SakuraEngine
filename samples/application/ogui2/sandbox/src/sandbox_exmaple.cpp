#include <chrono>
#include "SkrGui/system/input/hit_test.hpp"
#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/dev/sandbox.hpp"
#include "SkrCore/platform/system.h"
#include "SkrProfile/profile.h"
#include "SkrGui/backend/device/window.hpp"
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_trigger.hpp"
#include "input_binding.hpp"
#include "SkrGui/system/input/pointer_event.hpp"
#include "counter_state.hpp"

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
    skr::log::LogConstants::gFlushBehavior = skr::log::LogFlushBehavior::kFlushImmediate;

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
    bool b_quit  = false;
    auto handler = skr_system_get_default_handler();
    handler->add_window_close_handler(
    +[](SWindowHandle window, void* pQuit) {
        bool& quit = *(bool*)pQuit;
        quit       = true;
    },
    &b_quit);
    handler->add_window_resize_handler(
    +[](SWindowHandle window, int32_t w, int32_t h, void* usr_data) {
        auto sandbox = reinterpret_cast<Sandbox*>(usr_data);
        sandbox->resize_window(w, h);
    },
    sandbox);

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
            handler->pump_messages(delta_sec);
            handler->process_messages(delta_sec);
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
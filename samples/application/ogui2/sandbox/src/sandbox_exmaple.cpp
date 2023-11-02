#include "SkrGui/system/input/hit_test.hpp"
#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/dev/sandbox.hpp"
#include "SkrRT/platform/system.h"
#include "SkrProfile/profile.h"
#include "SkrGui/backend/device/window.hpp"
#include "SkrInputSystem/input_system.hpp"
#include "SkrInputSystem/input_trigger.hpp"

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
        auto widget = SNewWidget(Stack)
        {
            SNewChild(p.children, Positioned)
            {
                p.positional.fill();
                p.child = SNewWidget(GridPaper){};
            };
            SNewChild(p.children, Positioned)
            {
                p.positional.anchor_LT(0.5_pct, 0).sized(400, 400).pivot({ 0.5, 0 });
                p.child = SNewWidget(Flex)
                {
                    p.cross_axis_alignment = ECrossAxisAlignment::Start;
                    p.main_axis_alignment  = EMainAxisAlignment::Center;
                    SNewChild(p.children, SizedBox)
                    {
                        p.size  = { 100, 300 };
                        p.child = SNewWidget(ColoredBox) { p.color = Color::Linear("#F00"); };
                    };
                    SNewChild(p.children, SizedBox)
                    {
                        p.size  = { 100, 200 };
                        p.child = SNewWidget(ColoredBox) { p.color = Color::Linear("#0F0"); };
                    };
                    SNewChild(p.children, SizedBox)
                    {
                        p.size  = { 100, 400 };
                        p.child = SNewWidget(ColoredBox) { p.color = Color::Linear("#00F"); };
                    };
                };
            };
            // SNewChild(p.children, Positioned)
            // {
            //     p.positional.fill();
            //     p.child = SNewWidget(ColorPicker){};
            // };
            SNewChild(p.children, Positioned)
            {
                p.positional.anchor_LT(0.5_pct, 10_px).pivot({ 0.5, 0 });
                p.child = SNewWidget(Text) { p.text = u8"Hello World!"; };
            };
        };
        sandbox->set_content(skr::make_not_null(widget));
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

        // build actions
        auto action  = input_system->create_input_action(skr::input::EValueType::kBool);
        auto trigger = input_system->create_trigger<skr::input::InputTriggerDown>();
        action->add_trigger(trigger);
        action->bind_event<bool>([](const bool& down) {
            SKR_LOG_INFO(u8"Key F pressed: %d", down);
        });

        auto action2  = input_system->create_input_action(skr::input::EValueType::kBool);
        auto trigger2 = input_system->create_trigger<skr::input::InputTriggerPressed>();
        action2->add_trigger(trigger2);
        action2->bind_event<bool>([sandbox](const bool& f2) {
            int x, y;
            skr_cursor_pos(&x, &y, ECursorCoordinate::CURSOR_COORDINATE_SCREEN);

            HitTestResult result;
            sandbox->hit_test(&result, { (float)x, (float)y });
            skr::string path_str;
            for (auto node : result.path())
            {
                path_str += node.target->get_record_type()->name();
                path_str += u8"->";
            }
            SKR_LOG_INFO(u8"%s", path_str.c_str());
        });

        // mapping action <-> device keys
        auto mapping    = input_system->create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_F);
        mapping->action = action;
        mapping_ctx->add_mapping(mapping);

        auto mapping2    = input_system->create_mapping<skr::input::InputMapping_MouseButton>(EMouseKey::MOUSE_KEY_LB);
        mapping2->action = action2;
        mapping_ctx->add_mapping(mapping2);
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
    while (!b_quit)
    {
        FrameMark;
        float delta = 1.f / 60.f;
        {
            SkrZoneScopedN("SystemEvents");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }
        {
            SkrZoneScopedN("InputSystem");
            skr::input::Input::GetInstance()->Tick();
            input_system->update(delta);
        }
        {
            SkrZoneScopedN("Sandbox");
            sandbox->update();
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
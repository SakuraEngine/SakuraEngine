#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_resource_service.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/dev/sandbox.hpp"
#include "platform/system.h"
#include "tracy/Tracy.hpp"
#include "SkrGui/backend/device/window.hpp"

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

    // create backends
    SkrNativeDevice* device = SkrNew<SkrNativeDevice>();
    device->init();
    ICanvasService* canvas_service = create_embedded_canvas_service();
    ITextService*   text_service   = create_embedded_text_service(device->resource_device());

    // create sandbox
    Sandbox* sandbox = SkrNew<Sandbox>(device, canvas_service, text_service);
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
            // SNewChild(p.children, Positioned)
            // {
            //     p.positional.anchor_LT(0.5_pct, 10_px).pivot({ 0.5, 0 });
            //     p.child = SNewWidget(Text) { p.text = u8"Hello World!"; };
            // };
        };
        sandbox->set_content(skr::make_not_null(widget));
    }

    // show window
    {
        WindowDesc desc = {};
        desc.size       = { 600, 600 };
        desc.pos        = { 300, 300 };
        desc.name       = u8"SkrGUI Sandbox";
        sandbox->show(desc);
    }

    // run application
    bool quit    = false;
    auto handler = skr_system_get_default_handler();
    while (!quit)
    {
        FrameMark;
        float delta = 1.f / 60.f;
        {
            ZoneScopedN("SystemEvents");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }
        {
            ZoneScopedN("Sandbox");
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
    destroy_embedded_text_service(make_not_null(text_service));
    destroy_embedded_canvas_service(make_not_null(canvas_service));
    device->shutdown();
    SkrDelete(device);

    return 0;
}
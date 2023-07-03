#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/resource/skr_resource_service.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/dev/sandbox.hpp"
#include "platform/system.h"
#include "tracy/Tracy.hpp"

int main(void)
{
    using namespace skr::gui;

    // create backends
    SkrNativeDevice* device = SkrNew<SkrNativeDevice>();
    device->init();
    ICanvasService* canvas_service = create_embedded_canvas_service();
    ITextService*   text_service = create_embedded_text_service(device->resource_device());

    // create sandbox
    Sandbox* sandbox = SkrNew<Sandbox>(device, canvas_service, text_service);

    // run application
    bool quit = false;
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
        // TODO. update sandbox
    }

    // finalize
    destroy_embedded_text_service(make_not_null(text_service));
    destroy_embedded_canvas_service(make_not_null(canvas_service));
    device->shutdown();
    SkrDelete(device);

    return 0;
}
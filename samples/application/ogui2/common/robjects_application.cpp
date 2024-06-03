#include "./robjects_application.h"
#include "SkrCore/memory/memory.h"

// bool initialize_robjects_application(robjects_application_t* app)
// {
//     if (!initialize_gdi_application(&app->gdi)) return false;
//     app->platform_window = skr::gui::SPlatformWindow::Import(app->gdi.gfx.window_handle);

//     // create window context
//     skr::gui::WindowContext::Descriptor ctx_desc;
//     ctx_desc.platform_window = app->platform_window;
//     // ctx_desc.gdi_device = app->gdi.device;
//     app->window_context = skr::gui::WindowContext::Create(&ctx_desc);

//     return true;
// }

// bool finalize_robjects_application(robjects_application_t* app)
// {
//     skr::gui::WindowContext::Free(app->window_context);
//     skr::gui::SPlatformWindow::Free(app->platform_window);
//     return finalize_gdi_application(&app->gdi);
// }

void export_something()
{
    // avoid archiving an empty library
}
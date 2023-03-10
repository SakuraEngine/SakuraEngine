#include "./elem_application.h"
#include "SkrGui/window_context.hpp"
#include "SkrGuiRenderer/gui_window.hpp"
#include "SkrGui/render_elements/render_window.hpp"
#include "platform/memory.h"

bool initialize_elem_application(elements_application_t* app)
{
    if (!initialize_gdi_application(&app->gdi)) return false;
    app->platform_window = skr::gui::SPlatformWindow::Import(app->gdi.gfx.window_handle);

    // create root window
    app->root_window = SkrNew<skr::gui::RenderWindow>(app->gdi.device);

    // create window context
    skr::gui::WindowContextDescriptor ctx_desc;
    ctx_desc.platform_window = app->platform_window;
    ctx_desc.gdi_device = app->gdi.device;
    ctx_desc.root_window = app->root_window;
    app->window_context = skr::gui::WindowContext::Create(&ctx_desc);

    return true;
}

bool finalize_elem_application(elements_application_t* app)
{
    skr::gui::WindowContext::Free(app->window_context);
    SkrDelete(app->root_window);
    skr::gui::SPlatformWindow::Free(app->platform_window);
    return finalize_gdi_application(&app->gdi);
}
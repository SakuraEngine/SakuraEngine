#pragma once
#include "./gdi_application.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, IPlatformWindow, skr_gui_platform_window)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, WindowContext, skr_gui_window_context)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, RenderWindow, skr_gui_render_window)

struct robjects_application_t
{
    gdi_application_t gdi;
    skr_gui_platform_window_id platform_window;
    skr_gui_window_context_id window_context;
    skr_gui_render_window_id root_window;
};

bool initialize_robjects_application(robjects_application_t* app);
bool finalize_robjects_application(robjects_application_t* app);

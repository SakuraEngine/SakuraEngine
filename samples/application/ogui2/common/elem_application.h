#pragma once
#include "./gdi_application.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, SWindowContext, skr_gui_window_context)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, RenderWindow, skr_gui_render_window)

struct elements_application_t
{
    gdi_application_t gdi;
    SWindowHandle window;
    skr_gui_window_context_id window_context;
    skr_gui_render_window_id root_window;
};


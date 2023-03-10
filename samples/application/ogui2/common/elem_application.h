#pragma once
#include "./gdi_application.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, SWindowContext, skr_gui_window_context)

struct elements_application_t
{
    gdi_application_t gdi;
    SWindowHandle window;
    struct skr_gui_window_context_t* window_context;
};
#pragma once
#include "platform/window.h"
#include "platform/input.h"
#include "skr_imgui.config.h"

SKR_IMGUI_API void skr_imgui_new_frame(SWindowHandle window, float delta_time);

#ifdef __cplusplus
    #include "render_graph/frontend/render_graph.hpp"
    #include "module/module_manager.hpp"

    class SKR_IMGUI_API SkrImGuiModule : public skr::IDynamicModule
    {
    public:
        virtual void on_load(int argc, char** argv) override;
        virtual void on_unload() override;
    };
#endif
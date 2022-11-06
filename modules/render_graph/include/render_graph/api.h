#pragma once
#ifdef __cplusplus
    #include "rg_config.h"
    #include "render_graph/frontend/render_graph.hpp"
    #include "module/module.hpp"

    class SKR_RENDER_GRAPH_API SkrRenderGraphModule : public skr::IDynamicModule
    {
    public:
        virtual void on_load(int argc, char** argv) override;
        virtual void on_unload() override;
    };
#endif
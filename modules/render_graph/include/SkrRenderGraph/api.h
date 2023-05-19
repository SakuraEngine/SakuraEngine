#pragma once
#ifdef __cplusplus
    #include "rg_config.h"
    #include "SkrRenderGraph/frontend/render_graph.hpp"
    #include "module/module.hpp"

    class SKR_RENDER_GRAPH_API SkrRenderGraphModule : public skr::IDynamicModule
    {
    public:
        virtual void on_load(int argc, char8_t** argv) override;
        virtual void on_unload() override;

        static constexpr uint32_t kMaxFramesInFlight = RG_MAX_FRAME_IN_FLIGHT;
    };
#endif
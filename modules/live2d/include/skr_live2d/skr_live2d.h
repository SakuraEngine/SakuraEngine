#pragma once
#include "SkrLive2D/skr_live2d.configure.h"

#ifdef __cplusplus
    #include "module/module_manager.hpp"

class SKR_LIVE2D_API SkrLive2DModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;
private:
    bool _framework_initialized = false;
};

#endif
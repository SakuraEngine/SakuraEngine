#pragma once
#include "SkrLive2D/module.configure.h"

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

const float kLive2DViewScale = 1.0f;
const float kLive2DViewMaxScale = 2.0f;
const float kLive2DViewMinScale = 0.8f;

const float kLive2DViewLogicalLeft = -1.0f;
const float kLive2DViewLogicalRight = 1.0f;
const float kLive2DViewLogicalBottom = -1.0f;
const float kLive2DViewLogicalTop = -1.0f;

const float kLive2DViewLogicalMaxLeft = -2.0f;
const float kLive2DViewLogicalMaxRight = 2.0f;
const float kLive2DViewLogicalMaxBottom = -2.0f;
const float kLive2DViewLogicalMaxTop = 2.0f;

#endif
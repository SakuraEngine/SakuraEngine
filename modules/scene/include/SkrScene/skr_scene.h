#pragma once
#include "SkrScene/module.configure.h"
#include "scene.h"

#ifdef __cplusplus
    #include "SkrRT/module/module.hpp"
    
class SKR_SCENE_API SkrSceneModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    static SkrSceneModule* Get();
};
#endif
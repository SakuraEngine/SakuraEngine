#pragma once
#include "SkrScene/module.configure.h"
#include "scene.h"

#ifdef __cplusplus
    #include "module/module_manager.hpp"
    
class SKR_SCENE_API SkrSceneModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    static SkrSceneModule* Get();
};
#endif
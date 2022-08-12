#pragma once
#include "SkrImageCoder/skr_image_coder.configure.h"

#ifdef __cplusplus
    #include "module/module_manager.hpp"

class SKR_IMAGE_CODER_API SkrImageCoderModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;
};

#endif
#include "SkrModule/module.hpp"

class SkrGuiModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override {}
    virtual void on_unload() override {}
};


IMPLEMENT_DYNAMIC_MODULE(SkrGuiModule, SkrGui);
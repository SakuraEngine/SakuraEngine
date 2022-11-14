#include "SkrShaderCompiler/module.configure.h"
#include "module/module.hpp"
#include "utils/log.h"

struct SKR_SHADER_COMPILER_API SShaderCompilerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        for (auto&& [name, event] : on_load_events)
        {
            // SKR_LOG_DEBUG("ShaderCompilerModule event %s invoked", name.c_str());
            event();
        }
    }

    virtual void on_unload() override
    {

    }
    static eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> on_load_events;
};

IMPLEMENT_DYNAMIC_MODULE(SShaderCompilerModule, SkrShaderCompiler);
eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> SShaderCompilerModule::on_load_events = {};

SKR_SHADER_COMPILER_EXTERN_C
void Util_ShaderCompilerEventOnLoad(const char* name, eastl::function<void()> event)
{
    SShaderCompilerModule::on_load_events.push_back(eastl::make_pair(name, event));
}
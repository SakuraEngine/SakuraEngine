#include "SkrShaderCompiler/module.configure.h"
#include "module/module.hpp"
#include "utils/log.h"

struct SKR_SHADER_COMPILER_API SShaderCompilerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        for (auto&& [name, load_event] : on_load_events)
        {
            // SKR_LOG_DEBUG("ShaderCompilerModule load event %s invoked", name.c_str());
            load_event();
        }
    }

    virtual void on_unload() override
    {
        for (auto&& [name, unload_event] : on_unload_events)
        {
            // SKR_LOG_DEBUG("ShaderCompilerModule unload event %s invoked", name.c_str());
            unload_event();
        }
    }
    static eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> on_load_events;
    static eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> on_unload_events;
};

IMPLEMENT_DYNAMIC_MODULE(SShaderCompilerModule, SkrShaderCompiler);
eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> SShaderCompilerModule::on_load_events = {};
eastl::vector<eastl::pair<eastl::string, eastl::function<void()>>> SShaderCompilerModule::on_unload_events = {};

namespace skd
{
void Util_ShaderCompilerEventOnLoad(const char* name, const eastl::function<void()>& event)
{
    SShaderCompilerModule::on_load_events.emplace_back(name, event);
}

void Util_ShaderCompilerEventOnUnload(const char* name, const eastl::function<void()>& event)
{
    SShaderCompilerModule::on_unload_events.emplace_back(name, event);
}
} // end namespace skd